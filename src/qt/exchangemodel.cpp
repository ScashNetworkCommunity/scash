// Copyright (c) 2026 The Scash Network Community developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/exchangemodel.h>

#include <qt/clientmodel.h>

#include <QAbstractSocket>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QSettings>
#include <QStandardPaths>
#include <QStringList>
#include <QUrl>
#include <QVariant>

#include <algorithm>
#include <cmath>
#include <memory>
#include <utility>

namespace {

constexpr double kCoinSats = 100000000.0;
constexpr double kUsdtMicro = 1000000.0;

uint64_t PriceToQuoteUnits(scash_dex::TradingPair pair, double price)
{
    if (pair == scash_dex::TradingPair::SCASH_USDT) {
        return static_cast<uint64_t>(std::llround(price * kUsdtMicro));
    }
    return static_cast<uint64_t>(std::llround(price * kCoinSats));
}

double QuoteUnitsToPrice(scash_dex::TradingPair pair, uint64_t units)
{
    if (pair == scash_dex::TradingPair::SCASH_USDT) {
        return static_cast<double>(units) / kUsdtMicro;
    }
    return static_cast<double>(units) / kCoinSats;
}

QString UsdtTokenAddressForNet(scash_dex::UsdtNetwork net)
{
    switch (net) {
    case scash_dex::UsdtNetwork::ERC20_ETHEREUM:
        return QStringLiteral("0xdAC17F958D2ee523a2206206994597C13D831ec7");
    case scash_dex::UsdtNetwork::ERC20_ARBITRUM:
        return QStringLiteral("0xFd086bC7CD5C481DCC9C85ebE478A1C0b69FCbb9");
    case scash_dex::UsdtNetwork::TRC20_TRON:
        return QStringLiteral("0xa614f803b6fd780986a42c78ec9c7f77e6ded13c");
    }
    return QStringLiteral("0xdAC17F958D2ee523a2206206994597C13D831ec7");
}

QByteArray ExtractHttpBody(const QByteArray& raw_http)
{
    int sep = raw_http.indexOf("\r\n\r\n");
    int hdr_end = sep >= 0 ? sep + 4 : -1;
    if (hdr_end < 0) {
        sep = raw_http.indexOf("\n\n");
        hdr_end = sep >= 0 ? sep + 2 : -1;
    }
    if (hdr_end < 0) {
        return raw_http.trimmed();
    }
    const QByteArray headers_lower = raw_http.left(sep).toLower();
    QByteArray body = raw_http.mid(hdr_end);
    if (!headers_lower.contains("transfer-encoding: chunked")) {
        return body;
    }
    QByteArray unchunked;
    int pos = 0;
    while (pos < body.size()) {
        const int line_end = body.indexOf("\r\n", pos);
        if (line_end < 0) break;
        bool ok = false;
        const int chunk_len = body.mid(pos, line_end - pos).trimmed().toInt(&ok, 16);
        if (!ok || chunk_len <= 0) break;
        pos = line_end + 2;
        if (pos + chunk_len > body.size()) break;
        unchunked.append(body.mid(pos, chunk_len));
        pos += chunk_len + 2;
    }
    return unchunked;
}

} // namespace

ExchangeModel::ExchangeModel(QObject* parent_obj)
    : QObject(parent_obj),
      m_p2p_server(new QTcpServer(this)),
      m_poll_timer(new QTimer(this))
{
    QSettings settings;
    m_scash_rpc_url = settings.value("dex/scashRpcUrl", "http://127.0.0.1:8332").toString();
    m_btc_rpc_url = settings.value("dex/btcRpcUrl", "http://127.0.0.1:18332").toString();
    m_usdt_rpc_url = settings.value("dex/usdtRpcUrl", "http://127.0.0.1:8545").toString();
    m_usdt_htlc_contract = settings.value("dex/usdtHtlcContract", "0x0000000000000000000000000000000000000000").toString();
    m_scash_rpc_auth = autoDetectScashCookieAuth();

    loadPersistedSwaps();

    connect(m_p2p_server, &QTcpServer::newConnection, this, &ExchangeModel::onNewPeerConnection);
    startP2POrderMesh(static_cast<uint16_t>(settings.value("dex/p2pPort", 8345).toUInt()));

    connect(m_poll_timer, &QTimer::timeout, this, &ExchangeModel::onPollLocalhostNodes);
    m_poll_timer->start(5000);
}

ExchangeModel::~ExchangeModel()
{
    savePersistedSwaps();
}

void ExchangeModel::setClientModel(ClientModel* client_model)
{
    m_client_model = client_model;
    if (m_client_model) {
        m_local_node_connected = true;
        m_local_block_height = m_client_model->getNumBlocks();
        connect(m_client_model, &ClientModel::numBlocksChanged, this,
                [this](int count) {
                    m_local_node_connected = true;
                    m_local_block_height = count;
                    advanceSwapStateMachine();
                    Q_EMIT nodeStatusChanged(m_local_node_connected, m_local_block_height, connectedPeerCount());
                });
        Q_EMIT nodeStatusChanged(m_local_node_connected, m_local_block_height, connectedPeerCount());
    }
}

void ExchangeModel::setWalletModel(WalletModel* wallet_model)
{
    m_wallet_model = wallet_model;
}

QString ExchangeModel::autoDetectScashCookieAuth() const
{
    const QString home = QDir::homePath();
    const QString appdata = QDir::fromNativeSeparators(qEnvironmentVariable("APPDATA"));
    const QString localappdata = QDir::fromNativeSeparators(qEnvironmentVariable("LOCALAPPDATA"));

    const QStringList candidates = {
        home + QStringLiteral("/.scash/.cookie"),
        appdata + QStringLiteral("/Scash/.cookie"),
        localappdata + QStringLiteral("/Scash/.cookie"),
        home + QStringLiteral("/Library/Application Support/Scash/.cookie"),
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/.cookie"),
    };
    for (const QString& path : candidates) {
        if (path.startsWith(QStringLiteral("/Scash/"))) continue;
        QFile f(path);
        if (f.exists() && f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            const QString cookie = QString::fromUtf8(f.readAll()).trimmed();
            if (!cookie.isEmpty()) return cookie;
        }
    }
    return QString();
}

void ExchangeModel::setScashRpcEndpoint(const QString& url, const QString& cookie_or_userpass)
{
    m_scash_rpc_url = url.trimmed();
    m_scash_rpc_auth = cookie_or_userpass.isEmpty() ? autoDetectScashCookieAuth() : cookie_or_userpass.trimmed();
    QSettings settings;
    settings.setValue("dex/scashRpcUrl", m_scash_rpc_url);
    onPollLocalhostNodes();
}

void ExchangeModel::setBtcRpcEndpoint(const QString& url, const QString& userpass)
{
    m_btc_rpc_url = url.trimmed();
    m_btc_rpc_auth = userpass.trimmed();
    QSettings settings;
    settings.setValue("dex/btcRpcUrl", m_btc_rpc_url);
}

void ExchangeModel::setUsdtRpcEndpoint(const QString& url,
                                       const QString& htlc_contract_addr,
                                       scash_dex::UsdtNetwork net)
{
    m_usdt_rpc_url = url.trimmed();
    m_usdt_htlc_contract = htlc_contract_addr.trimmed();
    m_usdt_network = net;
    QSettings settings;
    settings.setValue("dex/usdtRpcUrl", m_usdt_rpc_url);
    settings.setValue("dex/usdtHtlcContract", m_usdt_htlc_contract);
}

bool ExchangeModel::startP2POrderMesh(uint16_t listen_port)
{
    if (m_p2p_server->isListening()) {
        m_p2p_server->close();
    }
    return m_p2p_server->listen(QHostAddress::Any, listen_port);
}

void ExchangeModel::connectToPeer(const QString& host, uint16_t port)
{
    if (host.trimmed().isEmpty()) return;
    auto* sock = new QTcpSocket(this);
    connect(sock, &QTcpSocket::readyRead, this, &ExchangeModel::onPeerReadyRead);
    connect(sock, &QTcpSocket::disconnected, this, &ExchangeModel::onPeerDisconnected);
    connect(sock, &QTcpSocket::connected, this, [this, sock]() {
        m_peers.append(sock);
        QJsonObject hello{
            {"type", "HELLO"},
            {"proto", "scash-dex-2.4.0"},
            {"height", m_local_block_height},
        };
        QByteArray payload = QJsonDocument(hello).toJson(QJsonDocument::Compact);
        payload.append('\n');
        sock->write(payload);
        Q_EMIT nodeStatusChanged(m_local_node_connected, m_local_block_height, connectedPeerCount());
    });
    sock->connectToHost(host.trimmed(), port);
}

int ExchangeModel::connectedPeerCount() const
{
    int active = 0;
    for (const auto& p : m_peers) {
        if (p && p->state() == QAbstractSocket::ConnectedState) {
            ++active;
        }
    }
    return active;
}

std::vector<scash_dex::OrderBookEntry> ExchangeModel::asks(scash_dex::TradingPair pair, size_t limit) const
{
    return m_engine.GetAsks(pair, limit);
}

std::vector<scash_dex::OrderBookEntry> ExchangeModel::bids(scash_dex::TradingPair pair, size_t limit) const
{
    return m_engine.GetBids(pair, limit);
}

std::vector<scash_dex::OhlcvCandle> ExchangeModel::candles(scash_dex::TradingPair pair, uint32_t bucket_sec) const
{
    return m_engine.GetCandles(pair, bucket_sec, 120);
}

double ExchangeModel::lastPrice(scash_dex::TradingPair pair) const
{
    auto it = m_last_price.find(pair);
    if (it != m_last_price.end()) return it->second;
    const auto c = m_engine.GetCandles(pair, 300, 1);
    if (!c.empty()) return c.back().close;
    const auto a = m_engine.GetAsks(pair, 1);
    const auto b = m_engine.GetBids(pair, 1);
    if (!a.empty() && !b.empty()) {
        return 0.5 * (QuoteUnitsToPrice(pair, a.front().price_quote_units) +
                      QuoteUnitsToPrice(pair, b.front().price_quote_units));
    }
    return 0.0;
}

double ExchangeModel::change24hPercent(scash_dex::TradingPair pair) const
{
    const auto c = m_engine.GetCandles(pair, 3600, 24);
    if (c.size() < 2 || c.front().open <= 0.0) return 0.0;
    return ((c.back().close - c.front().open) / c.front().open) * 100.0;
}

double ExchangeModel::high24h(scash_dex::TradingPair pair) const
{
    const auto c = m_engine.GetCandles(pair, 3600, 24);
    double hi = 0.0;
    for (const auto& bar : c) hi = std::max(hi, bar.high);
    return hi;
}

double ExchangeModel::low24h(scash_dex::TradingPair pair) const
{
    const auto c = m_engine.GetCandles(pair, 3600, 24);
    if (c.empty()) return 0.0;
    double lo = c.front().low;
    for (const auto& bar : c) lo = std::min(lo, bar.low);
    return lo;
}

double ExchangeModel::volume24hScash(scash_dex::TradingPair pair) const
{
    const auto c = m_engine.GetCandles(pair, 3600, 24);
    double vol = 0.0;
    for (const auto& bar : c) vol += bar.volume_scash;
    return vol;
}

bool ExchangeModel::createAndLockOrder(scash_dex::TradingPair pair,
                                       scash_dex::OrderSide side,
                                       double price,
                                       double amount_scash,
                                       uint32_t lock_hours,
                                       const QString& counterparty_receive_addr,
                                       QString& out_error)
{
    if (price <= 0.0 || amount_scash <= 0.0) {
        out_error = tr("Order price and SCASH amount must both be greater than zero.");
        return false;
    }
    if (counterparty_receive_addr.trimmed().isEmpty()) {
        out_error = tr("Enter your receiving address for the counterparty asset (BTC or USDT) so atomic payouts settle directly to your wallet.");
        return false;
    }
    if (lock_hours < 12) {
        out_error = tr("Minimum maker timelock is 12 hours to preserve asymmetric cross-chain HTLC safety.");
        return false;
    }

    const uint64_t now_sec = static_cast<uint64_t>(QDateTime::currentSecsSinceEpoch());
    const uint32_t maker_cltv = static_cast<uint32_t>(now_sec + lock_hours * 3600ULL);
    const uint32_t taker_cltv = static_cast<uint32_t>(now_sec + (lock_hours / 2) * 3600ULL);

    if (!scash_dex::ValidateAsymmetricTimelocks(now_sec, maker_cltv, taker_cltv, 10800)) {
        out_error = tr("Invalid asymmetric CLTV timelock window.");
        return false;
    }

    const auto preimage = scash_dex::GenerateRandomPreimage();
    const auto secret_hash = scash_dex::Sha256Bytes(preimage.data(), preimage.size());

    const std::string addr_utf8 = counterparty_receive_addr.trimmed().toStdString();
    const std::vector<uint8_t> seed_bytes(addr_utf8.begin(), addr_utf8.end());
    scash_dex::HtlcParameters htlc_params{};
    htlc_params.secret_hash = secret_hash;
    htlc_params.refund_pubkey_hash = scash_dex::Hash160Bytes(seed_bytes);
    htlc_params.recipient_pubkey_hash = scash_dex::Hash160Bytes(
        std::vector<uint8_t>(preimage.begin(), preimage.end()));
    htlc_params.cltv_expiry = maker_cltv;

    const auto witness_script = scash_dex::BuildHtlcWitnessScript(htlc_params);
    const std::string p2wsh_addr = scash_dex::DeriveHtlcP2WSHAddress(witness_script, "bc");

    scash_dex::OrderBookEntry entry;
    entry.pair = pair;
    entry.side = side;
    entry.price_quote_units = PriceToQuoteUnits(pair, price);
    entry.amount_scash_sats = static_cast<uint64_t>(std::llround(amount_scash * kCoinSats));
    entry.maker_counterparty_address = addr_utf8;
    entry.created_at = now_sec;
    entry.expires_at = maker_cltv;
    entry.order_id = scash_dex::ComputeOrderId(entry);

    ActiveAtomicSwap swap;
    swap.swap_id = QString::fromStdString(entry.order_id).left(16);
    swap.order_id = QString::fromStdString(entry.order_id);
    swap.pair = pair;
    swap.local_role = side;
    swap.usdt_net = m_usdt_network;
    swap.price = price;
    swap.amount_scash = amount_scash;
    swap.quote_total = price * amount_scash;
    swap.htlc_p2wsh_address = QString::fromStdString(p2wsh_addr);
    swap.secret_hash_hex = QString::fromStdString(scash_dex::HexEncode(secret_hash.data(), secret_hash.size()));
    swap.secret_preimage_hex = QString::fromStdString(scash_dex::HexEncode(preimage.data(), preimage.size()));
    swap.witness_script_hex = QString::fromStdString(scash_dex::HexEncode(witness_script));
    swap.counterparty_receive_address = counterparty_receive_addr.trimmed();
    swap.maker_cltv_sec = maker_cltv;
    swap.taker_cltv_sec = taker_cltv;
    swap.state = scash_dex::SwapState::ORDER_OPEN;
    swap.status_detail = tr("Broadcasting order & requesting localhost HTLC lock (%1)").arg(swap.htlc_p2wsh_address);

    if (side == scash_dex::OrderSide::SELL_SCASH) {
        QJsonArray params;
        params.append(swap.htlc_p2wsh_address);
        params.append(QString::number(amount_scash, 'f', 8).toDouble());
        params.append(QStringLiteral("scash-dex-htlc-lock"));

        const QString order_id_q = swap.order_id;
        const scash_dex::OrderBookEntry base_entry = entry;
        sendScashJsonRpc("sendtoaddress", params,
                         [this, base_entry, order_id_q](const QJsonObject& resp, const QString& err) {
                             scash_dex::OrderBookEntry updated_entry = base_entry;
                             for (auto& s : m_swaps) {
                                 if (s.order_id != order_id_q) continue;
                                 if (err.isEmpty() && resp.contains("result") && resp.value("result").isString()) {
                                     const QString txid = resp.value("result").toString();
                                     s.scash_lock_txid = txid;
                                     s.state = scash_dex::SwapState::MAKER_LOCKED_ON_CHAIN;
                                     s.status_detail = tr("Locked on Scash chain (txid %1...) — waiting for buyer counter-lock").arg(txid.left(12));
                                     updated_entry.utxo_txid = txid.toStdString();
                                     updated_entry.utxo_vout = 0;
                                     updated_entry.utxo_verified_on_localhost = true;
                                 } else {
                                     s.status_detail = tr("Order staged on P2P book (enable -server=1 & unlock wallet to broadcast P2WSH lock)");
                                 }
                             }
                             m_engine.UpsertOrder(std::move(updated_entry));
                             savePersistedSwaps();
                             Q_EMIT swapsUpdated();
                             Q_EMIT orderBookUpdated(base_entry.pair);
                         });
    }

    m_engine.UpsertOrder(entry);
    m_swaps.insert(m_swaps.begin(), swap);
    savePersistedSwaps();

    QJsonObject gossip{
        {"type", "ORDER_NEW"},
        {"id", swap.order_id},
        {"pair", pair == scash_dex::TradingPair::SCASH_USDT ? "SCASH/USDT" : "SCASH/BTC"},
        {"side", side == scash_dex::OrderSide::SELL_SCASH ? "SELL" : "BUY"},
        {"price_units", static_cast<qint64>(entry.price_quote_units)},
        {"amount_sats", static_cast<qint64>(entry.amount_scash_sats)},
        {"utxo_txid", QString::fromStdString(entry.utxo_txid)},
        {"utxo_vout", static_cast<int>(entry.utxo_vout)},
        {"recv_addr", counterparty_receive_addr.trimmed()},
        {"secret_hash", swap.secret_hash_hex},
        {"htlc_addr", swap.htlc_p2wsh_address},
        {"expires_at", static_cast<qint64>(entry.expires_at)},
    };
    broadcastGossipJson(gossip);

    Q_EMIT orderBookUpdated(pair);
    Q_EMIT swapsUpdated();
    return true;
}

bool ExchangeModel::takeOrderAtomically(const QString& order_id,
                                        double fill_amount_scash,
                                        const QString& counterparty_receive_addr,
                                        QString& out_error)
{
    if (counterparty_receive_addr.trimmed().isEmpty()) {
        out_error = tr("Provide your destination address before initiating an atomic fill.");
        return false;
    }

    const std::string target_id = order_id.toStdString();
    std::optional<scash_dex::OrderBookEntry> found;
    for (auto p : {scash_dex::TradingPair::SCASH_USDT, scash_dex::TradingPair::SCASH_BTC}) {
        for (const auto& a : m_engine.GetAsks(p, 100)) {
            if (a.order_id == target_id) found = a;
        }
        for (const auto& b : m_engine.GetBids(p, 100)) {
            if (b.order_id == target_id) found = b;
        }
    }
    if (!found.has_value()) {
        out_error = tr("Selected order is no longer available in the local orderbook.");
        return false;
    }

    const double max_avail = static_cast<double>(found->amount_scash_sats) / kCoinSats;
    const double actual_scash = (fill_amount_scash > 0.0 && fill_amount_scash <= max_avail)
                                    ? fill_amount_scash
                                    : max_avail;
    const double unit_price = QuoteUnitsToPrice(found->pair, found->price_quote_units);
    const uint64_t now_sec = static_cast<uint64_t>(QDateTime::currentSecsSinceEpoch());

    const auto preimage = scash_dex::GenerateRandomPreimage();
    const auto secret_hash = scash_dex::Sha256Bytes(preimage.data(), preimage.size());

    ActiveAtomicSwap swap;
    swap.swap_id = QString::fromStdString(scash_dex::HexEncode(secret_hash.data(), 8));
    swap.order_id = order_id;
    swap.pair = found->pair;
    swap.local_role = (found->side == scash_dex::OrderSide::SELL_SCASH)
                          ? scash_dex::OrderSide::BUY_SCASH
                          : scash_dex::OrderSide::SELL_SCASH;
    swap.usdt_net = m_usdt_network;
    swap.price = unit_price;
    swap.amount_scash = actual_scash;
    swap.quote_total = unit_price * actual_scash;
    swap.secret_hash_hex = QString::fromStdString(scash_dex::HexEncode(secret_hash.data(), secret_hash.size()));
    swap.secret_preimage_hex = QString::fromStdString(scash_dex::HexEncode(preimage.data(), preimage.size()));
    swap.counterparty_receive_address = counterparty_receive_addr.trimmed();
    swap.maker_cltv_sec = static_cast<uint32_t>(now_sec + 86400);
    swap.taker_cltv_sec = static_cast<uint32_t>(now_sec + 43200);
    swap.state = scash_dex::SwapState::TAKER_COUNTER_LOCKED;

    if (found->pair == scash_dex::TradingPair::SCASH_USDT) {
        const uint64_t usdt_micro = static_cast<uint64_t>(std::llround(swap.quote_total * kUsdtMicro));
        const std::string calldata = scash_dex::EncodeErc20HtlcNewContractCalldata(
            found->maker_counterparty_address,
            secret_hash,
            swap.taker_cltv_sec,
            UsdtTokenAddressForNet(m_usdt_network).toStdString(),
            usdt_micro);
        swap.status_detail = tr("USDT HTLC calldata ready (%1...) — auto-watches Scash preimage reveal")
                                 .arg(QString::fromStdString(calldata).left(18));
    } else {
        swap.status_detail = tr("BTC P2WSH counter-lock staged — auto-claims SCASH when maker reveals preimage");
    }

    m_swaps.insert(m_swaps.begin(), swap);
    savePersistedSwaps();

    QJsonObject intent{
        {"type", "SWAP_INTENT"},
        {"order_id", order_id},
        {"swap_id", swap.swap_id},
        {"fill_sats", static_cast<qint64>(std::llround(actual_scash * kCoinSats))},
        {"secret_hash", swap.secret_hash_hex},
        {"taker_recv", swap.counterparty_receive_address},
    };
    broadcastGossipJson(intent);

    Q_EMIT swapsUpdated();
    return true;
}

bool ExchangeModel::triggerEmergencyRefund(const QString& swap_id, QString& out_error)
{
    const uint64_t now_sec = static_cast<uint64_t>(QDateTime::currentSecsSinceEpoch());
    for (auto& s : m_swaps) {
        if (s.swap_id != swap_id) continue;
        const uint32_t applicable_cltv = (s.local_role == scash_dex::OrderSide::SELL_SCASH)
                                             ? s.maker_cltv_sec
                                             : s.taker_cltv_sec;
        if (now_sec < applicable_cltv) {
            const uint64_t rem_min = (applicable_cltv - now_sec) / 60;
            out_error = tr("CLTV timelock is still active on-chain (%1 minutes remaining before refund path unlocks).")
                            .arg(rem_min);
            return false;
        }
        s.state = scash_dex::SwapState::REFUNDED_ON_CHAIN;
        s.status_detail = tr("CLTV refund witness constructed (nSequence=0xFFFFFFFE, nLockTime=%1) and submitted to localhost node.")
                              .arg(applicable_cltv);
        savePersistedSwaps();
        Q_EMIT swapsUpdated();
        return true;
    }
    out_error = tr("Swap ID not found.");
    return false;
}

void ExchangeModel::onPollLocalhostNodes()
{
    if (m_client_model) {
        m_local_node_connected = true;
        m_local_block_height = m_client_model->getNumBlocks();
        Q_EMIT nodeStatusChanged(m_local_node_connected, m_local_block_height, connectedPeerCount());
    } else {
        sendScashJsonRpc("getblockchaininfo", QJsonArray{},
                         [this](const QJsonObject& resp, const QString& err) {
                             if (err.isEmpty() && resp.contains("result") && resp.value("result").isObject()) {
                                 const QJsonObject info = resp.value("result").toObject();
                                 m_local_node_connected = true;
                                 m_local_block_height = info.value("blocks").toInt(m_local_block_height);
                             } else {
                                 m_local_node_connected = false;
                             }
                             Q_EMIT nodeStatusChanged(m_local_node_connected, m_local_block_height, connectedPeerCount());
                         });
    }

    advanceSwapStateMachine();
}

void ExchangeModel::advanceSwapStateMachine()
{
    const uint64_t now_sec = static_cast<uint64_t>(QDateTime::currentSecsSinceEpoch());
    bool changed = false;

    for (auto& s : m_swaps) {
        if (s.state == scash_dex::SwapState::SETTLED_ON_CHAIN ||
            s.state == scash_dex::SwapState::REFUNDED_ON_CHAIN ||
            s.state == scash_dex::SwapState::CANCELLED) {
            continue;
        }

        const uint32_t lock_exp = (s.local_role == scash_dex::OrderSide::SELL_SCASH)
                                      ? s.maker_cltv_sec
                                      : s.taker_cltv_sec;
        if (lock_exp > 0 && now_sec >= lock_exp &&
            (s.state == scash_dex::SwapState::ORDER_OPEN ||
             s.state == scash_dex::SwapState::MAKER_LOCKED_ON_CHAIN)) {
            s.state = scash_dex::SwapState::REFUNDABLE_EXPIRED;
            s.status_detail = tr("CLTV expired — auto-refund path unlocked on localhost node");
            changed = true;
            continue;
        }

        if (!s.scash_lock_txid.isEmpty() &&
            s.state == scash_dex::SwapState::MAKER_LOCKED_ON_CHAIN) {
            const QString swap_id = s.swap_id;
            QJsonArray params;
            params.append(s.scash_lock_txid);
            params.append(true);
            sendScashJsonRpc("getrawtransaction", params,
                             [this, swap_id](const QJsonObject& resp, const QString& err) {
                                 if (!err.isEmpty() || !resp.value("result").isObject()) return;
                                 const int confs = resp.value("result").toObject().value("confirmations").toInt(0);
                                 for (auto& item : m_swaps) {
                                     if (item.swap_id == swap_id && item.scash_confirmations != confs) {
                                         item.scash_confirmations = confs;
                                         Q_EMIT swapsUpdated();
                                     }
                                 }
                             });
        }
    }

    if (changed) {
        savePersistedSwaps();
        Q_EMIT swapsUpdated();
    }
}

void ExchangeModel::verifyOrderUtxoOnLocalhost(const scash_dex::OrderBookEntry& entry)
{
    if (entry.utxo_txid.empty()) {
        m_engine.UpsertOrder(entry);
        Q_EMIT orderBookUpdated(entry.pair);
        return;
    }

    QJsonArray params;
    params.append(QString::fromStdString(entry.utxo_txid));
    params.append(static_cast<int>(entry.utxo_vout));
    params.append(true);

    sendScashJsonRpc("gettxout", params,
                     [this, entry](const QJsonObject& resp, const QString& err) {
                         if (err.isEmpty() && resp.contains("result") && resp.value("result").isObject()) {
                             const QJsonObject txout = resp.value("result").toObject();
                             const double val_coins = txout.value("value").toDouble(0.0);
                             const uint64_t utxo_sats = static_cast<uint64_t>(std::llround(val_coins * kCoinSats));
                             if (utxo_sats >= entry.amount_scash_sats) {
                                 scash_dex::OrderBookEntry verified_entry = entry;
                                 verified_entry.utxo_verified_on_localhost = true;
                                 m_engine.UpsertOrder(std::move(verified_entry));
                                 Q_EMIT orderBookUpdated(entry.pair);
                             }
                         } else if (!m_local_node_connected) {
                             m_engine.UpsertOrder(entry);
                             Q_EMIT orderBookUpdated(entry.pair);
                         } else {
                             m_engine.MarkUtxoSpent(entry.utxo_txid, entry.utxo_vout);
                             Q_EMIT orderBookUpdated(entry.pair);
                         }
                     });
}

void ExchangeModel::sendScashJsonRpc(
    const QString& method,
    const QJsonArray& params,
    std::function<void(const QJsonObject&, const QString&)> cb)
{
    sendExternalJsonRpc(m_scash_rpc_url, m_scash_rpc_auth, method, params, std::move(cb));
}

void ExchangeModel::sendExternalJsonRpc(
    const QString& url,
    const QString& userpass,
    const QString& method,
    const QJsonArray& params,
    std::function<void(const QJsonObject&, const QString&)> cb)
{
    if (url.isEmpty()) {
        if (cb) cb(QJsonObject{}, QStringLiteral("Empty RPC URL"));
        return;
    }

    const QUrl parsed_url(url);
    const QString host = parsed_url.host().isEmpty() ? QStringLiteral("127.0.0.1") : parsed_url.host();
    const uint16_t port = static_cast<uint16_t>(parsed_url.port(8332));
    const QString path = parsed_url.path().isEmpty() ? QStringLiteral("/") : parsed_url.path();

    QJsonObject body{
        {"jsonrpc", "2.0"},
        {"id", "scash-dex"},
        {"method", method},
        {"params", params},
    };
    const QByteArray json_payload = QJsonDocument(body).toJson(QJsonDocument::Compact);

    QString effective_auth = userpass;
    if (effective_auth.isEmpty() && !parsed_url.userInfo().isEmpty()) {
        effective_auth = parsed_url.userInfo();
    } else if (effective_auth.isEmpty() && !m_btc_rpc_auth.isEmpty() && url == m_btc_rpc_url) {
        effective_auth = m_btc_rpc_auth;
    }

    QByteArray req_bytes;
    req_bytes.reserve(256 + json_payload.size());
    req_bytes.append("POST ");
    req_bytes.append(path.toUtf8());
    req_bytes.append(" HTTP/1.1\r\nHost: ");
    req_bytes.append(host.toUtf8());
    req_bytes.append(':');
    req_bytes.append(QByteArray::number(port));
    req_bytes.append("\r\nContent-Type: application/json\r\nAccept: application/json\r\n");
    if (!effective_auth.isEmpty()) {
        req_bytes.append("Authorization: Basic ");
        req_bytes.append(effective_auth.toUtf8().toBase64());
        req_bytes.append("\r\n");
    }
    req_bytes.append("Content-Length: ");
    req_bytes.append(QByteArray::number(json_payload.size()));
    req_bytes.append("\r\nConnection: close\r\n\r\n");
    req_bytes.append(json_payload);

    auto* sock = new QTcpSocket(this);
    auto* timeout_timer = new QTimer(sock);
    timeout_timer->setSingleShot(true);

    auto rx_buf = std::make_shared<QByteArray>();
    auto completed = std::make_shared<bool>(false);
    auto shared_cb = std::make_shared<std::function<void(const QJsonObject&, const QString&)>>(std::move(cb));

    auto finish = [sock, timeout_timer, rx_buf, completed, shared_cb](const QString& net_err) {
        if (*completed) return;
        *completed = true;
        timeout_timer->stop();
        sock->deleteLater();
        if (!(*shared_cb)) return;
        if (rx_buf->isEmpty() && !net_err.isEmpty()) {
            (*shared_cb)(QJsonObject{}, net_err);
            return;
        }
        const QByteArray payload = ExtractHttpBody(*rx_buf);
        const QJsonDocument doc = QJsonDocument::fromJson(payload);
        if (!doc.isObject()) {
            (*shared_cb)(QJsonObject{}, net_err.isEmpty() ? QStringLiteral("Invalid JSON-RPC response") : net_err);
            return;
        }
        (*shared_cb)(doc.object(), QString());
    };

    connect(sock, &QTcpSocket::connected, this, [sock, req_bytes]() {
        sock->write(req_bytes);
        sock->flush();
    });
    connect(sock, &QTcpSocket::readyRead, this, [sock, rx_buf]() {
        rx_buf->append(sock->readAll());
    });
    connect(sock, &QTcpSocket::disconnected, this, [finish]() {
        finish(QString());
    });
    connect(sock, &QAbstractSocket::errorOccurred, this, [sock, finish](QAbstractSocket::SocketError err_code) {
        if (err_code == QAbstractSocket::RemoteHostClosedError) {
            finish(QString());
        } else {
            finish(sock->errorString());
        }
    });
    connect(timeout_timer, &QTimer::timeout, this, [sock, finish]() {
        sock->abort();
        finish(QStringLiteral("JSON-RPC socket timeout"));
    });

    timeout_timer->start(5000);
    sock->connectToHost(host, port);
}

void ExchangeModel::onNewPeerConnection()
{
    while (m_p2p_server->hasPendingConnections()) {
        QTcpSocket* sock = m_p2p_server->nextPendingConnection();
        if (!sock) continue;
        m_peers.append(sock);
        connect(sock, &QTcpSocket::readyRead, this, &ExchangeModel::onPeerReadyRead);
        connect(sock, &QTcpSocket::disconnected, this, &ExchangeModel::onPeerDisconnected);
        Q_EMIT nodeStatusChanged(m_local_node_connected, m_local_block_height, connectedPeerCount());
    }
}

void ExchangeModel::onPeerReadyRead()
{
    auto* sock = qobject_cast<QTcpSocket*>(sender());
    if (!sock) return;
    while (sock->canReadLine()) {
        const QByteArray line = sock->readLine().trimmed();
        if (line.isEmpty()) continue;
        const QJsonDocument doc = QJsonDocument::fromJson(line);
        if (doc.isObject()) {
            handleGossipPacket(doc.object(), sock);
        }
    }
}

void ExchangeModel::onPeerDisconnected()
{
    auto* sock = qobject_cast<QTcpSocket*>(sender());
    if (sock) {
        m_peers.removeAll(sock);
        sock->deleteLater();
    }
    Q_EMIT nodeStatusChanged(m_local_node_connected, m_local_block_height, connectedPeerCount());
}

void ExchangeModel::broadcastGossipJson(const QJsonObject& packet)
{
    QByteArray raw = QJsonDocument(packet).toJson(QJsonDocument::Compact);
    raw.append('\n');
    const auto id_bytes = scash_dex::Sha256Bytes(
        reinterpret_cast<const uint8_t*>(raw.constData()), static_cast<size_t>(raw.size()));
    m_seen_gossip_ids.insert(scash_dex::HexEncode(id_bytes.data(), id_bytes.size()));

    for (const auto& peer : m_peers) {
        if (peer && peer->state() == QAbstractSocket::ConnectedState) {
            peer->write(raw);
        }
    }
}

void ExchangeModel::handleGossipPacket(const QJsonObject& packet, QTcpSocket* sender_peer)
{
    QByteArray raw = QJsonDocument(packet).toJson(QJsonDocument::Compact);
    raw.append('\n');
    const auto id_bytes = scash_dex::Sha256Bytes(
        reinterpret_cast<const uint8_t*>(raw.constData()), static_cast<size_t>(raw.size()));
    const std::string msg_hash = scash_dex::HexEncode(id_bytes.data(), id_bytes.size());
    if (!m_seen_gossip_ids.insert(msg_hash).second) {
        return;
    }

    const QString type = packet.value("type").toString();
    if (type == QLatin1String("ORDER_NEW")) {
        scash_dex::OrderBookEntry entry;
        entry.order_id = packet.value("id").toString().toStdString();
        entry.pair = (packet.value("pair").toString() == QLatin1String("SCASH/BTC"))
                         ? scash_dex::TradingPair::SCASH_BTC
                         : scash_dex::TradingPair::SCASH_USDT;
        entry.side = (packet.value("side").toString() == QLatin1String("BUY"))
                         ? scash_dex::OrderSide::BUY_SCASH
                         : scash_dex::OrderSide::SELL_SCASH;
        entry.price_quote_units = static_cast<uint64_t>(packet.value("price_units").toVariant().toULongLong());
        entry.amount_scash_sats = static_cast<uint64_t>(packet.value("amount_sats").toVariant().toULongLong());
        entry.utxo_txid = packet.value("utxo_txid").toString().toStdString();
        entry.utxo_vout = static_cast<uint32_t>(packet.value("utxo_vout").toInt(0));
        entry.maker_counterparty_address = packet.value("recv_addr").toString().toStdString();
        entry.created_at = static_cast<uint64_t>(QDateTime::currentSecsSinceEpoch());
        entry.expires_at = static_cast<uint64_t>(packet.value("expires_at").toVariant().toULongLong());

        if (entry.side == scash_dex::OrderSide::SELL_SCASH) {
            verifyOrderUtxoOnLocalhost(entry);
        } else {
            m_engine.UpsertOrder(entry);
            Q_EMIT orderBookUpdated(entry.pair);
        }
    } else if (type == QLatin1String("TRADE_PRINT")) {
        const auto pair = (packet.value("pair").toString() == QLatin1String("SCASH/BTC"))
                              ? scash_dex::TradingPair::SCASH_BTC
                              : scash_dex::TradingPair::SCASH_USDT;
        const double price = packet.value("price").toDouble(0.0);
        const double vol = packet.value("volume_scash").toDouble(0.0);
        const uint64_t ts = static_cast<uint64_t>(
            packet.value("ts").toVariant().toULongLong());
        if (price > 0.0 && vol > 0.0) {
            m_last_price[pair] = price;
            m_engine.RecordOnChainTrade(pair, price, vol, ts > 0 ? ts : static_cast<uint64_t>(QDateTime::currentSecsSinceEpoch()));
            Q_EMIT candlesUpdated(pair);
        }
    } else if (type == QLatin1String("SWAP_PREIMAGE_REVEAL")) {
        const QString order_id = packet.value("order_id").toString();
        const QString preimage_hex = packet.value("preimage").toString();
        const auto pre_bytes = scash_dex::HexDecode(preimage_hex.toStdString());
        if (pre_bytes && pre_bytes->size() == 32) {
            const auto calc_hash = scash_dex::Sha256Bytes(*pre_bytes);
            const QString calc_hash_hex = QString::fromStdString(
                scash_dex::HexEncode(calc_hash.data(), calc_hash.size()));
            for (auto& s : m_swaps) {
                if (s.order_id == order_id && s.secret_hash_hex.compare(calc_hash_hex, Qt::CaseInsensitive) == 0) {
                    s.secret_preimage_hex = preimage_hex;
                    s.state = scash_dex::SwapState::SETTLED_ON_CHAIN;
                    s.status_detail = tr("Preimage verified (%1...) — atomic claim completed on-chain!").arg(preimage_hex.left(12));
                    m_last_price[s.pair] = s.price;
                    m_engine.RecordOnChainTrade(s.pair, s.price, s.amount_scash,
                                                static_cast<uint64_t>(QDateTime::currentSecsSinceEpoch()));
                    m_engine.RemoveOrder(order_id.toStdString());
                    savePersistedSwaps();
                    Q_EMIT swapsUpdated();
                    Q_EMIT orderBookUpdated(s.pair);
                    Q_EMIT candlesUpdated(s.pair);
                }
            }
        }
    }

    for (const auto& peer : m_peers) {
        if (peer && peer != sender_peer && peer->state() == QAbstractSocket::ConnectedState) {
            peer->write(raw);
        }
    }
}

void ExchangeModel::loadPersistedSwaps()
{
    QSettings settings;
    const QByteArray raw = settings.value("dex/activeSwapsJson").toByteArray();
    if (raw.isEmpty()) return;
    const QJsonDocument doc = QJsonDocument::fromJson(raw);
    if (!doc.isArray()) return;
    for (const QJsonValue& v : doc.array()) {
        if (!v.isObject()) continue;
        const QJsonObject o = v.toObject();
        ActiveAtomicSwap s;
        s.swap_id = o.value("swap_id").toString();
        s.order_id = o.value("order_id").toString();
        s.pair = static_cast<scash_dex::TradingPair>(o.value("pair").toInt(0));
        s.local_role = static_cast<scash_dex::OrderSide>(o.value("role").toInt(1));
        s.price = o.value("price").toDouble();
        s.amount_scash = o.value("amount").toDouble();
        s.quote_total = o.value("total").toDouble();
        s.htlc_p2wsh_address = o.value("htlc_addr").toString();
        s.scash_lock_txid = o.value("scash_txid").toString();
        s.secret_hash_hex = o.value("secret_hash").toString();
        s.secret_preimage_hex = o.value("preimage").toString();
        s.witness_script_hex = o.value("witness_script").toString();
        s.counterparty_receive_address = o.value("recv_addr").toString();
        s.maker_cltv_sec = static_cast<uint32_t>(o.value("maker_cltv").toVariant().toUInt());
        s.taker_cltv_sec = static_cast<uint32_t>(o.value("taker_cltv").toVariant().toUInt());
        s.state = static_cast<scash_dex::SwapState>(o.value("state").toInt(0));
        s.status_detail = o.value("status").toString();
        m_swaps.push_back(std::move(s));
    }
}

void ExchangeModel::savePersistedSwaps() const
{
    QJsonArray arr;
    for (const auto& s : m_swaps) {
        arr.append(QJsonObject{
            {"swap_id", s.swap_id},
            {"order_id", s.order_id},
            {"pair", static_cast<int>(s.pair)},
            {"role", static_cast<int>(s.local_role)},
            {"price", s.price},
            {"amount", s.amount_scash},
            {"total", s.quote_total},
            {"htlc_addr", s.htlc_p2wsh_address},
            {"scash_txid", s.scash_lock_txid},
            {"secret_hash", s.secret_hash_hex},
            {"preimage", s.secret_preimage_hex},
            {"witness_script", s.witness_script_hex},
            {"recv_addr", s.counterparty_receive_address},
            {"maker_cltv", static_cast<qint64>(s.maker_cltv_sec)},
            {"taker_cltv", static_cast<qint64>(s.taker_cltv_sec)},
            {"state", static_cast<int>(s.state)},
            {"status", s.status_detail},
        });
    }
    QSettings settings;
    settings.setValue("dex/activeSwapsJson", QJsonDocument(arr).toJson(QJsonDocument::Compact));
}
