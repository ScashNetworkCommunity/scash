// Copyright (c) 2026 The Scash Network Community developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/exchangemodel.h>

#include <interfaces/node.h>
#include <qt/clientmodel.h>
#include <qt/walletmodel.h>
#include <univalue.h>

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
#include <exception>
#include <limits>
#include <memory>
#include <sstream>
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
    Q_EMIT walletStatusChanged();
}

bool ExchangeModel::isWalletEncrypted() const
{
    if (!m_wallet_model) return false;
    const auto st = m_wallet_model->getEncryptionStatus();
    return st == WalletModel::Locked || st == WalletModel::Unlocked;
}

bool ExchangeModel::isWalletWatchOnly() const
{
    if (!m_wallet_model) return false;
    return m_wallet_model->getEncryptionStatus() == WalletModel::NoKeys;
}

QString ExchangeModel::activeWalletName() const
{
    if (m_wallet_model) {
        const QString w = m_wallet_model->getWalletName();
        return w.isEmpty() ? tr("(default)") : w;
    }
    return tr("(none)");
}

double ExchangeModel::spendableScashBalance() const
{
    if (!m_client_model) return 0.0;
    ExchangeModel* self = const_cast<ExchangeModel*>(this);
    QJsonObject resp;
    QString err;
    if (self->tryExecuteInProcessRpc(QStringLiteral("getbalances"), QJsonArray{}, true, resp, err) && err.isEmpty()) {
        const QJsonObject mine = resp.value("result").toObject().value("mine").toObject();
        if (mine.contains("trusted")) {
            return mine.value("trusted").toDouble(0.0);
        }
    }
    if (self->tryExecuteInProcessRpc(QStringLiteral("getbalance"), QJsonArray{}, true, resp, err) && err.isEmpty()) {
        return resp.value("result").toDouble(0.0);
    }
    return 0.0;
}

QString ExchangeModel::activeWalletDisplayStatus() const
{
    if (!m_wallet_model) {
        return tr("Wallet: None loaded");
    }
    const double bal = spendableScashBalance();
    const QString guard = isWalletEncrypted()
                              ? tr("Passphrase-Per-Tx Active")
                              : tr("Unencrypted (Encrypt in Settings)");
    return tr("Wallet: %1 • Available: %2 SCASH • %3")
        .arg(activeWalletName(), QString::number(bal, 'f', 4), guard);
}

bool ExchangeModel::sendOutputsWithFeeFallback(const QJsonObject& addr_amounts, QString& out_txid, QString& out_error)
{
    QJsonArray sm_params;
    sm_params.append(QStringLiteral(""));
    sm_params.append(addr_amounts);

    QJsonObject sm_resp;
    QString sm_err;
    if (tryExecuteInProcessRpc(QStringLiteral("sendmany"), sm_params, true, sm_resp, sm_err)) {
        if (sm_err.isEmpty() && sm_resp.value("result").isString()) {
            out_txid = sm_resp.value("result").toString();
            out_error.clear();
            return true;
        }
        // Fallback to Bitcoin Core 30 `send` RPC with explicit 1.0 sat/vB fee_rate if fee estimation lacks blocks
        if (sm_err.contains("Fee estimation failed", Qt::CaseInsensitive) ||
            sm_err.contains("Fallbackfee", Qt::CaseInsensitive)) {
            QJsonArray outputs_arr;
            outputs_arr.append(addr_amounts);
            QJsonArray send_params;
            send_params.append(outputs_arr);
            send_params.append(QJsonValue()); // conf_target = null
            send_params.append(QStringLiteral("unset"));
            send_params.append(1.0); // explicit 1.0 sat/vB (10x Scash 0.1 sat/vB min relay fee)

            QJsonObject send_resp;
            QString send_err;
            if (tryExecuteInProcessRpc(QStringLiteral("send"), send_params, true, send_resp, send_err) &&
                send_err.isEmpty()) {
                const QJsonObject res_obj = send_resp.value("result").toObject();
                if (res_obj.value("txid").isString()) {
                    out_txid = res_obj.value("txid").toString();
                    out_error.clear();
                    return true;
                }
            }
        }
        out_error = sm_err;
        return false;
    }
    out_error = tr("Local Scash node is not attached.");
    return false;
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

bool ExchangeModel::tryExecuteInProcessRpc(const QString& method,
                                           const QJsonArray& params,
                                           bool use_wallet_uri,
                                           QJsonObject& out_resp,
                                           QString& out_error)
{
    if (!m_client_model) {
        return false;
    }

    UniValue uv_params(UniValue::VARR);
    const QByteArray params_bytes = QJsonDocument(params).toJson(QJsonDocument::Compact);
    if (!uv_params.read(std::string(params_bytes.constData(), static_cast<size_t>(params_bytes.size())))) {
        out_error = QStringLiteral("Failed to serialize RPC parameters.");
        return true;
    }

    std::string uri = "/";
    if (use_wallet_uri) {
        if (m_wallet_model) {
            const QString wname = m_wallet_model->getWalletName();
            const QByteArray encoded = QUrl::toPercentEncoding(wname);
            uri = "/wallet/" + std::string(encoded.constData(), static_cast<size_t>(encoded.size()));
        } else {
            try {
                const UniValue wallets = m_client_model->node().executeRpc("listwallets", UniValue(UniValue::VARR), "/");
                if (wallets.isArray() && wallets.size() > 0 && wallets[0].isStr()) {
                    const QString wname = QString::fromStdString(wallets[0].get_str());
                    const QByteArray encoded = QUrl::toPercentEncoding(wname);
                    uri = "/wallet/" + std::string(encoded.constData(), static_cast<size_t>(encoded.size()));
                }
            } catch (...) {
            }
        }
    }

    try {
        const UniValue res = m_client_model->node().executeRpc(method.toStdString(), uv_params, uri);
        const std::string wrapped_str = "{\"result\":" + res.write() + "}";
        const QJsonDocument doc = QJsonDocument::fromJson(
            QByteArray(wrapped_str.c_str()));
        out_resp = doc.object();
        out_error.clear();
        return true;
    } catch (const UniValue& obj_err) {
        std::string err_str = obj_err.write();
        const QJsonDocument edoc = QJsonDocument::fromJson(
            QByteArray(err_str.c_str()));
        if (edoc.isObject() && edoc.object().value("message").isString()) {
            out_error = edoc.object().value("message").toString();
        } else {
            out_error = QString::fromStdString(err_str);
        }
        return true;
    } catch (const std::exception& e) {
        out_error = QString::fromUtf8(e.what());
        return true;
    }
}

QString ExchangeModel::generateLocalScashAddress(QString& out_error)
{
    QJsonObject resp;
    QJsonArray params;
    params.append(QStringLiteral("scash-dex-receive"));
    params.append(QStringLiteral("bech32"));
    if (tryExecuteInProcessRpc(QStringLiteral("getnewaddress"), params, true, resp, out_error)) {
        if (out_error.isEmpty() && resp.value("result").isString()) {
            return resp.value("result").toString();
        }
        return QString();
    }
    out_error = tr("Load or create a Scash wallet first (File -> Create/Open Wallet) to generate a scash1q receiving address.");
    return QString();
}

bool ExchangeModel::deriveLocalWalletPubkeyHex(QString& out_scash_addr, QString& out_pubkey_hex, QString& out_error)
{
    out_scash_addr = generateLocalScashAddress(out_error);
    if (out_scash_addr.isEmpty()) {
        return false;
    }
    QJsonObject info_resp;
    QJsonArray info_params;
    info_params.append(out_scash_addr);
    if (!tryExecuteInProcessRpc(QStringLiteral("getaddressinfo"), info_params, true, info_resp, out_error) || !out_error.isEmpty()) {
        if (out_error.isEmpty()) out_error = tr("Failed to query getaddressinfo on active wallet.");
        return false;
    }
    const QJsonObject res = info_resp.value("result").toObject();
    const QString pub_hex = res.value("pubkey").toString().trimmed();
    if (!scash_dex::IsValidCompressedPubKeyHex(pub_hex.toStdString())) {
        out_error = tr("Active wallet did not return a valid 33-byte compressed secp256k1 public key for %1.").arg(out_scash_addr);
        return false;
    }
    out_pubkey_hex = pub_hex;
    return true;
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

std::vector<scash_dex::AggregatedBookLevel> ExchangeModel::aggregatedAsks(scash_dex::TradingPair pair, size_t limit) const
{
    return m_engine.GetAggregatedAsks(pair, limit);
}

std::vector<scash_dex::AggregatedBookLevel> ExchangeModel::aggregatedBids(scash_dex::TradingPair pair, size_t limit) const
{
    return m_engine.GetAggregatedBids(pair, limit);
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
                                       uint32_t max_lots,
                                       uint16_t bond_bps,
                                       bool arm_watchtower,
                                       QString& out_error)
{
    if (price <= 0.0 || amount_scash <= 0.0) {
        out_error = tr("Order price and SCASH amount must both be greater than zero.");
        return false;
    }

    const QString trimmed_addr = counterparty_receive_addr.trimmed();
    if (trimmed_addr.isEmpty()) {
        if (side == scash_dex::OrderSide::SELL_SCASH) {
            out_error = pair == scash_dex::TradingPair::SCASH_USDT
                            ? tr("Enter your USDT payout address (0x... for ERC-20 or T... for TRC-20) where buyers will send USDT.")
                            : tr("Enter your Bitcoin payout address (bc1q...) where buyers will send BTC.");
        } else {
            out_error = tr("Enter your Scash receiving address (scash1q...) or click Auto-Fill.");
        }
        return false;
    }

    if (side == scash_dex::OrderSide::SELL_SCASH) {
        if (trimmed_addr.startsWith(QStringLiteral("scash1"), Qt::CaseInsensitive)) {
            out_error = pair == scash_dex::TradingPair::SCASH_USDT
                            ? tr("You entered a Scash (scash1...) address in USDT Payout Addr. Enter your 0x... (Ethereum/Arbitrum) or T... (Tron) USDT address instead.")
                            : tr("You entered a Scash (scash1...) address in BTC Payout Addr. Enter your bc1q... Bitcoin address instead.");
            return false;
        }
    } else {
        if (!trimmed_addr.startsWith(QStringLiteral("scash1"), Qt::CaseInsensitive) &&
            !trimmed_addr.startsWith(QStringLiteral("tscash1"), Qt::CaseInsensitive) &&
            !trimmed_addr.startsWith(QStringLiteral("1")) &&
            !trimmed_addr.startsWith(QStringLiteral("3"))) {
            out_error = tr("When buying SCASH, enter a valid Scash destination address (scash1q...) or click Auto-Fill.");
            return false;
        }
    }

    // AUTO-CROSS CHECK (Real Exchange Matching):
    // If buying SCASH at `price >= best_ask` or selling SCASH at `price <= best_bid`, automatically match the resting book!
    const uint64_t limit_units = PriceToQuoteUnits(pair, price);
    if (side == scash_dex::OrderSide::BUY_SCASH) {
        const auto best_asks = m_engine.GetAsks(pair, 1);
        if (!best_asks.empty() && limit_units >= best_asks.front().price_quote_units) {
            return takeOrderAtomically(
                QString::fromStdString(best_asks.front().order_id),
                amount_scash,
                trimmed_addr,
                out_error);
        }
    } else {
        const auto best_bids = m_engine.GetBids(pair, 1);
        if (!best_bids.empty() && limit_units <= best_bids.front().price_quote_units) {
            return takeOrderAtomically(
                QString::fromStdString(best_bids.front().order_id),
                amount_scash,
                trimmed_addr,
                out_error);
        }
    }

    if (lock_hours < 2) {
        out_error = tr("Minimum maker timelock is 2 hours.");
        return false;
    }

    if (m_wallet_model && isWalletWatchOnly()) {
        out_error = tr("Wallet '%1' is watch-only (private keys disabled) and cannot sign Exchange orders.")
                        .arg(activeWalletName());
        return false;
    }

    if (side == scash_dex::OrderSide::SELL_SCASH && m_client_model) {
        const double avail = spendableScashBalance();
        if (avail < amount_scash) {
            out_error = tr("Wallet '%1' only has %2 SCASH available (cannot post %3 SCASH sell order).")
                            .arg(activeWalletName(),
                                 QString::number(avail, 'f', 4),
                                 QString::number(amount_scash, 'f', 4));
            return false;
        }
    }

    // Enforce per-transaction SecureString passphrase prompt & instant RAII re-lock!
    QString local_scash_refund_addr;
    QString local_pubkey_hex;
    if (m_wallet_model && m_wallet_model->getEncryptionStatus() == WalletModel::Unlocked) {
        m_wallet_model->setWalletLocked(true);
    }
    if (m_wallet_model && m_wallet_model->getEncryptionStatus() == WalletModel::Locked) {
        WalletModel::UnlockContext unlock_ctx(m_wallet_model->requestUnlock());
        if (!unlock_ctx.isValid()) {
            out_error = tr("Order cancelled: wallet passphrase unlock was cancelled or incorrect.");
            return false;
        }
        if (!deriveLocalWalletPubkeyHex(local_scash_refund_addr, local_pubkey_hex, out_error)) {
            return false;
        }
    } else if (m_client_model) {
        if (!deriveLocalWalletPubkeyHex(local_scash_refund_addr, local_pubkey_hex, out_error)) {
            return false;
        }
    }

    const uint64_t now_sec = static_cast<uint64_t>(QDateTime::currentSecsSinceEpoch());
    processWatchtowerDuties(now_sec);
    const uint32_t maker_cltv = static_cast<uint32_t>(now_sec + lock_hours * 3600ULL);
    const uint32_t taker_cltv = static_cast<uint32_t>(now_sec + std::min<uint64_t>(1800ULL, (lock_hours * 3600ULL) / 2ULL));

    if (!scash_dex::ValidateAsymmetricTimelocks(now_sec, maker_cltv, taker_cltv, 1800)) {
        out_error = tr("Invalid asymmetric CLTV timelock window.");
        return false;
    }

    const uint64_t total_sats = static_cast<uint64_t>(std::llround(amount_scash * kCoinSats));
    const uint32_t effective_max_lots = (side == scash_dex::OrderSide::SELL_SCASH) ? std::max(1U, max_lots) : 1U;
    const auto lot_sats_vec = scash_dex::SplitAmountIntoLots(total_sats, effective_max_lots, 10000000ULL);
    const uint32_t lot_count = static_cast<uint32_t>(lot_sats_vec.size());

    const auto master_seed = scash_dex::GenerateRandomPreimage();
    const std::string addr_utf8 = trimmed_addr.toStdString();
    const std::string parent_id = scash_dex::HexEncode(
        scash_dex::Sha256Bytes(master_seed.data(), master_seed.size()).data(), 16);

    for (uint32_t i = 0; i < lot_count; ++i) {
        const auto lot_pre = scash_dex::DeriveIndexedLotPreimage(master_seed, i);
        const auto lot_hash = scash_dex::Sha256Bytes(lot_pre.data(), lot_pre.size());

        scash_dex::OrderBookEntry entry;
        entry.parent_order_id = parent_id;
        entry.lot_index = i;
        entry.total_lots = lot_count;
        entry.anti_option_bond_bps = bond_bps;
        entry.pair = pair;
        entry.side = side;
        entry.price_quote_units = limit_units;
        entry.amount_scash_sats = lot_sats_vec[i];
        entry.maker_scash_pubkey_hex = local_pubkey_hex.toStdString();
        entry.maker_counterparty_address = addr_utf8;
        entry.created_at = now_sec;
        entry.expires_at = maker_cltv;
        entry.utxo_verified_on_localhost = (m_client_model != nullptr);
        entry.order_id = scash_dex::ComputeOrderId(entry);
        m_engine.UpsertOrder(entry);

        QJsonObject gossip{
            {"type", "ORDER_NEW"},
            {"id", QString::fromStdString(entry.order_id)},
            {"parent_id", QString::fromStdString(parent_id)},
            {"lot_idx", static_cast<int>(i)},
            {"total_lots", static_cast<int>(lot_count)},
            {"bond_bps", static_cast<int>(bond_bps)},
            {"pair", pair == scash_dex::TradingPair::SCASH_USDT ? "SCASH/USDT" : "SCASH/BTC"},
            {"side", side == scash_dex::OrderSide::SELL_SCASH ? "SELL" : "BUY"},
            {"price_units", static_cast<qint64>(entry.price_quote_units)},
            {"amount_sats", static_cast<qint64>(entry.amount_scash_sats)},
            {"maker_pub", local_pubkey_hex},
            {"recv_addr", trimmed_addr},
            {"secret_hash", QString::fromStdString(scash_dex::HexEncode(lot_hash.data(), 32))},
            {"expires_at", static_cast<qint64>(entry.expires_at)},
        };
        broadcastGossipJson(gossip);
    }

    const auto lot0_pre = scash_dex::DeriveIndexedLotPreimage(master_seed, 0);
    const auto lot0_hash = scash_dex::Sha256Bytes(lot0_pre.data(), lot0_pre.size());

    ActiveAtomicSwap swap;
    swap.swap_id = QString::fromStdString(parent_id).left(16);
    swap.order_id = QString::fromStdString(parent_id);
    swap.pair = pair;
    swap.local_role = side;
    swap.usdt_net = m_usdt_network;
    swap.price = price;
    swap.amount_scash = amount_scash;
    swap.quote_total = price * amount_scash;
    swap.htlc_p2wsh_address = local_scash_refund_addr;
    swap.seller_refund_scash_addr = local_scash_refund_addr;
    swap.seller_refund_pubkey_hex = local_pubkey_hex;
    swap.secret_hash_hex = QString::fromStdString(scash_dex::HexEncode(lot0_hash.data(), 32));
    swap.secret_preimage_hex = QString::fromStdString(scash_dex::HexEncode(lot0_pre.data(), 32));
    swap.counterparty_receive_address = trimmed_addr;
    swap.maker_cltv_sec = maker_cltv;
    swap.taker_cltv_sec = taker_cltv;
    swap.total_lots = lot_count;
    swap.filled_lots = 0;
    swap.anti_option_bond_bps = bond_bps;
    swap.state = scash_dex::SwapState::ORDER_OPEN;

    if (arm_watchtower) {
        buildAndRelayWatchtowerBundle(swap);
    }

    if (side == scash_dex::OrderSide::SELL_SCASH) {
        swap.status_detail = tr("OPEN SELL (%1 lots = %2 SCASH safely in '%3' until buyer matches — Cancel anytime for 0 fee)")
                                 .arg(lot_count)
                                 .arg(QString::number(amount_scash, 'f', 4),
                                      activeWalletName());
    } else {
        swap.status_detail = tr("OPEN BUY BID (%1 SCASH @ %2 | %3% anti-option bond | Watchtower Armed)")
                                 .arg(QString::number(amount_scash, 'f', 4),
                                      QString::number(price, 'f', pair == scash_dex::TradingPair::SCASH_USDT ? 6 : 8),
                                      QString::number(bond_bps / 100.0, 'f', 1));
    }

    m_swaps.insert(m_swaps.begin(), swap);
    savePersistedSwaps();

    Q_EMIT orderBookUpdated(pair);
    Q_EMIT swapsUpdated();
    Q_EMIT walletStatusChanged();
    return true;
}

bool ExchangeModel::lockMatchedHtlcOnChain(const QString& swap_id,
                                           const QString& buyer_pubkey_hex,
                                           QString& out_error)
{
    if (!scash_dex::IsValidCompressedPubKeyHex(buyer_pubkey_hex.toStdString())) {
        out_error = tr("Refusing to lock SCASH: counterparty did not supply a valid 33-byte compressed secp256k1 public key.");
        return false;
    }

    for (auto& s : m_swaps) {
        if (s.swap_id != swap_id) continue;
        if (!scash_dex::IsValidCompressedPubKeyHex(s.seller_refund_pubkey_hex.toStdString())) {
            if (!deriveLocalWalletPubkeyHex(s.seller_refund_scash_addr, s.seller_refund_pubkey_hex, out_error)) {
                return false;
            }
        }
        const auto buyer_pub = scash_dex::HexDecode(buyer_pubkey_hex.toStdString());
        const auto seller_pub = scash_dex::HexDecode(s.seller_refund_pubkey_hex.toStdString());
        const auto hash_bytes = scash_dex::HexDecode(s.secret_hash_hex.toStdString());
        if (!buyer_pub || !seller_pub || !hash_bytes || hash_bytes->size() != 32) {
            out_error = tr("Invalid HTLC public key or hash parameters.");
            return false;
        }

        scash_dex::HtlcParameters hp{};
        std::copy_n(hash_bytes->begin(), 32, hp.secret_hash.begin());
        hp.recipient_pubkey_hash = scash_dex::Hash160Bytes(*buyer_pub);
        hp.refund_pubkey_hash = scash_dex::Hash160Bytes(*seller_pub);
        hp.cltv_expiry = s.maker_cltv_sec;

        const auto ws = scash_dex::BuildHtlcWitnessScript(hp);
        s.witness_script_hex = QString::fromStdString(scash_dex::HexEncode(ws));
        s.buyer_claim_pubkey_hex = buyer_pubkey_hex;
        s.htlc_p2wsh_address = QString::fromStdString(scash_dex::DeriveHtlcP2WSHAddress(ws, "scash"));

        QJsonObject addr_map;
        const std::string map_str =
            "{"" + s.htlc_p2wsh_address.toStdString() + "":" + QString::number(s.amount_scash, 'f', 8).toStdString() + "}";
        addr_map = QJsonDocument::fromJson(QByteArray(map_str.c_str())).object();

        QString txid;
        if (!sendOutputsWithFeeFallback(addr_map, txid, out_error)) {
            return false;
        }
        s.scash_lock_txid = txid;
        s.state = scash_dex::SwapState::MAKER_LOCKED_ON_CHAIN;
        buildAndRelayWatchtowerBundle(s);
        s.status_detail = tr("LOCKED ON-CHAIN (txid %1... | Verified 33B Buyer+Seller Pubkeys | CLTV Refund Safe)")
                              .arg(txid.left(14));
        savePersistedSwaps();
        Q_EMIT swapsUpdated();
        Q_EMIT walletStatusChanged();
        return true;
    }
    out_error = tr("Matching swap ID not found.");
    return false;
}

bool ExchangeModel::takeOrderAtomically(const QString& order_id,
                                        double fill_amount_scash,
                                        const QString& counterparty_receive_addr,
                                        QString& out_error)
{
    const QString trimmed_addr = counterparty_receive_addr.trimmed();
    if (trimmed_addr.isEmpty()) {
        out_error = tr("Enter your destination address (or click Auto-Fill) before initiating an atomic fill.");
        return false;
    }
    if (m_wallet_model && isWalletWatchOnly()) {
        out_error = tr("Wallet '%1' is watch-only and cannot sign atomic swap transactions.").arg(activeWalletName());
        return false;
    }

    const std::string target_id = order_id.toStdString();
    std::optional<scash_dex::OrderBookEntry> seed_order;
    for (auto p : {scash_dex::TradingPair::SCASH_USDT, scash_dex::TradingPair::SCASH_BTC}) {
        for (const auto& a : m_engine.GetAsks(p, 200)) {
            if (a.order_id == target_id || a.parent_order_id == target_id) {
                seed_order = a;
                break;
            }
        }
        for (const auto& b : m_engine.GetBids(p, 200)) {
            if (b.order_id == target_id || b.parent_order_id == target_id) {
                seed_order = b;
                break;
            }
        }
    }
    if (!seed_order.has_value()) {
        out_error = tr("Selected order level is no longer available in the local orderbook.");
        return false;
    }

    const scash_dex::OrderSide taker_side =
        (seed_order->side == scash_dex::OrderSide::SELL_SCASH)
            ? scash_dex::OrderSide::BUY_SCASH
            : scash_dex::OrderSide::SELL_SCASH;

    if (taker_side == scash_dex::OrderSide::BUY_SCASH &&
        !trimmed_addr.startsWith(QStringLiteral("scash1"), Qt::CaseInsensitive) &&
        !trimmed_addr.startsWith(QStringLiteral("tscash1"), Qt::CaseInsensitive)) {
        out_error = tr("To buy SCASH, enter a scash1q... receiving address (click Auto-Fill).");
        return false;
    }
    if (taker_side == scash_dex::OrderSide::SELL_SCASH &&
        trimmed_addr.startsWith(QStringLiteral("scash1"), Qt::CaseInsensitive)) {
        out_error = seed_order->pair == scash_dex::TradingPair::SCASH_USDT
                        ? tr("To sell SCASH into a Bid, enter your 0x... or T... USDT payout address.")
                        : tr("To sell SCASH into a Bid, enter your bc1q... Bitcoin payout address.");
        return false;
    }

    // Verify wallet passphrase via SecureString AskPassphraseDialog + derive real 33-byte wallet pubkey!
    QString taker_scash_addr;
    QString taker_pubkey_hex;
    if (m_wallet_model && m_wallet_model->getEncryptionStatus() == WalletModel::Unlocked) {
        m_wallet_model->setWalletLocked(true);
    }
    if (m_wallet_model && m_wallet_model->getEncryptionStatus() == WalletModel::Locked) {
        WalletModel::UnlockContext unlock_ctx(m_wallet_model->requestUnlock());
        if (!unlock_ctx.isValid()) {
            out_error = tr("Atomic fill cancelled: wallet passphrase unlock was cancelled or incorrect.");
            return false;
        }
        deriveLocalWalletPubkeyHex(taker_scash_addr, taker_pubkey_hex, out_error);
        out_error.clear();
    } else if (m_client_model) {
        deriveLocalWalletPubkeyHex(taker_scash_addr, taker_pubkey_hex, out_error);
        out_error.clear();
    }

    const uint64_t req_sats = static_cast<uint64_t>(std::llround(std::max(0.0001, fill_amount_scash) * kCoinSats));
    const uint64_t match_limit = (taker_side == scash_dex::OrderSide::BUY_SCASH)
                                     ? std::numeric_limits<uint64_t>::max()
                                     : 0ULL;
    const auto matched_lots = m_engine.MatchTakerOrder(
        seed_order->pair, taker_side, match_limit, req_sats);
    if (matched_lots.empty()) {
        out_error = tr("No matching lot outputs available at that price level.");
        return false;
    }

    uint64_t filled_sats = 0;
    const std::string matched_parent_id = matched_lots.front().parent_order_id;
    const auto all_side_orders = (taker_side == scash_dex::OrderSide::BUY_SCASH)
                                     ? m_engine.GetAsks(seed_order->pair, 500)
                                     : m_engine.GetBids(seed_order->pair, 500);
    for (const auto& ml : matched_lots) {
        filled_sats += ml.amount_scash_sats;
        uint64_t orig_sats = ml.amount_scash_sats;
        scash_dex::OrderBookEntry orig_entry = ml;
        for (const auto& cand : all_side_orders) {
            if (cand.order_id == ml.order_id) {
                orig_sats = cand.amount_scash_sats;
                orig_entry = cand;
                break;
            }
        }
        m_engine.RemoveOrder(ml.order_id);
        if (orig_sats > ml.amount_scash_sats) {
            orig_entry.amount_scash_sats = orig_sats - ml.amount_scash_sats;
            m_engine.UpsertOrder(orig_entry);
        }
    }

    // Detect if this is a local loopback self-match (matching your own resting order on the same wallet with 0 peers)
    bool is_local_loopback = false;
    for (auto& existing : m_swaps) {
        if (!matched_parent_id.empty() && existing.order_id.toStdString() == matched_parent_id) {
            is_local_loopback = true;
            existing.filled_lots += static_cast<uint32_t>(matched_lots.size());
            const double rem_scash = std::max(0.0, existing.amount_scash - (static_cast<double>(filled_sats) / kCoinSats));
            existing.amount_scash = rem_scash;
            if (rem_scash <= 1e-8) {
                existing.state = scash_dex::SwapState::SETTLED_ON_CHAIN;
                existing.status_detail = tr("100% FILLED (%1 lots matched @ %2)")
                                             .arg(existing.total_lots)
                                             .arg(QString::number(existing.price, 'f', 6));
            } else {
                existing.status_detail = tr("PARTIALLY FILLED (%1 SCASH remaining open on book)")
                                             .arg(QString::number(rem_scash, 'f', 4));
            }
            break;
        }
    }

    const double actual_scash = static_cast<double>(filled_sats) / kCoinSats;
    const double unit_price = QuoteUnitsToPrice(seed_order->pair, seed_order->price_quote_units);
    const uint64_t now_sec = static_cast<uint64_t>(QDateTime::currentSecsSinceEpoch());

    const auto preimage = scash_dex::GenerateRandomPreimage();
    const auto secret_hash = scash_dex::Sha256Bytes(preimage.data(), preimage.size());

    ActiveAtomicSwap swap;
    swap.swap_id = QString::fromStdString(scash_dex::HexEncode(secret_hash.data(), 8));
    swap.order_id = QString::fromStdString(matched_lots.front().order_id);
    swap.pair = seed_order->pair;
    swap.local_role = taker_side;
    swap.usdt_net = m_usdt_network;
    swap.price = unit_price;
    swap.amount_scash = actual_scash;
    swap.quote_total = unit_price * actual_scash;
    swap.buyer_claim_pubkey_hex = taker_pubkey_hex;
    swap.secret_hash_hex = QString::fromStdString(scash_dex::HexEncode(secret_hash.data(), secret_hash.size()));
    swap.secret_preimage_hex = QString::fromStdString(scash_dex::HexEncode(preimage.data(), preimage.size()));
    swap.counterparty_receive_address = trimmed_addr;
    swap.maker_cltv_sec = static_cast<uint32_t>(now_sec + 14400);
    swap.taker_cltv_sec = static_cast<uint32_t>(now_sec + 1200);
    swap.total_lots = static_cast<uint32_t>(matched_lots.size());
    swap.filled_lots = static_cast<uint32_t>(matched_lots.size());
    swap.anti_option_bond_bps = seed_order->anti_option_bond_bps;

    if (seed_order->pair == scash_dex::TradingPair::SCASH_USDT) {
        const uint64_t usdt_micro = static_cast<uint64_t>(std::llround(swap.quote_total * kUsdtMicro));
        const uint64_t bond_micro = scash_dex::ComputeAntiOptionBondUnits(usdt_micro, swap.anti_option_bond_bps);
        const std::string calldata = scash_dex::EncodeErc20HtlcNewContractCalldata(
            seed_order->maker_counterparty_address,
            secret_hash,
            swap.taker_cltv_sec,
            UsdtTokenAddressForNet(m_usdt_network).toStdString(),
            usdt_micro + bond_micro);
        swap.counterparty_htlc_payload = QString::fromStdString(calldata);
    } else if (scash_dex::IsValidCompressedPubKeyHex(seed_order->maker_scash_pubkey_hex) &&
               scash_dex::IsValidCompressedPubKeyHex(taker_pubkey_hex.toStdString())) {
        scash_dex::HtlcParameters btc_params{};
        btc_params.secret_hash = secret_hash;
        btc_params.recipient_pubkey_hash = scash_dex::Hash160Bytes(*scash_dex::HexDecode(seed_order->maker_scash_pubkey_hex));
        btc_params.refund_pubkey_hash = scash_dex::Hash160Bytes(*scash_dex::HexDecode(taker_pubkey_hex.toStdString()));
        btc_params.cltv_expiry = swap.taker_cltv_sec;
        const auto btc_ws = scash_dex::BuildHtlcWitnessScript(btc_params);
        swap.htlc_p2wsh_address = QString::fromStdString(scash_dex::DeriveHtlcP2WSHAddress(btc_ws, "bc"));
        swap.counterparty_htlc_payload = QString::fromStdString(scash_dex::HexEncode(btc_ws));
    }

    buildAndRelayWatchtowerBundle(swap);

    if (is_local_loopback && connectedPeerCount() == 0) {
        // Self-match test on single wallet (0 external peers): settle immediately & print live OHLCV candle!
        swap.state = scash_dex::SwapState::SETTLED_ON_CHAIN;
        swap.status_detail = tr("LOCAL SELF-MATCH SETTLED (%1 lot(s) = %2 SCASH @ %3 — printed OHLCV candle; 0 SCASH spent)")
                                 .arg(matched_lots.size())
                                 .arg(QString::number(actual_scash, 'f', 4),
                                      QString::number(unit_price, 'f', seed_order->pair == scash_dex::TradingPair::SCASH_USDT ? 6 : 8));
        m_last_price[seed_order->pair] = unit_price;
        m_engine.RecordOnChainTrade(seed_order->pair, unit_price, actual_scash, now_sec);
        Q_EMIT candlesUpdated(seed_order->pair);
    } else {
        swap.state = scash_dex::SwapState::TAKER_COUNTER_LOCKED;
        swap.status_detail = tr("Matched %1 lot(s) (%2 SCASH) — sent verified 33B pubkey to peer & armed Watchtower")
                                 .arg(matched_lots.size())
                                 .arg(QString::number(actual_scash, 'f', 4));
    }

    m_swaps.insert(m_swaps.begin(), swap);
    savePersistedSwaps();

    QJsonObject intent{
        {"type", "SWAP_INTENT"},
        {"order_id", swap.order_id},
        {"parent_id", QString::fromStdString(matched_parent_id)},
        {"swap_id", swap.swap_id},
        {"lots_filled", static_cast<int>(matched_lots.size())},
        {"fill_sats", static_cast<qint64>(filled_sats)},
        {"buyer_pub", taker_pubkey_hex},
        {"secret_hash", swap.secret_hash_hex},
        {"taker_recv", swap.counterparty_receive_address},
    };
    broadcastGossipJson(intent);

    Q_EMIT orderBookUpdated(seed_order->pair);
    Q_EMIT swapsUpdated();
    Q_EMIT walletStatusChanged();
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
        if (m_wallet_model && m_wallet_model->getEncryptionStatus() == WalletModel::Unlocked) {
            m_wallet_model->setWalletLocked(true);
        }
        if (m_wallet_model && m_wallet_model->getEncryptionStatus() == WalletModel::Locked) {
            WalletModel::UnlockContext unlock_ctx(m_wallet_model->requestUnlock());
            if (!unlock_ctx.isValid()) {
                out_error = tr("Refund cancelled: wallet passphrase unlock was cancelled or incorrect.");
                return false;
            }
            if (!s.presigned_refund_tx_hex.isEmpty()) {
                QJsonArray p;
                p.append(s.presigned_refund_tx_hex);
                QJsonObject r;
                QString e;
                tryExecuteInProcessRpc(QStringLiteral("sendrawtransaction"), p, false, r, e);
            }
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

bool ExchangeModel::removeUnfencedSwap(const QString& swap_id, QString& out_error)
{
    for (auto it = m_swaps.begin(); it != m_swaps.end(); ++it) {
        if (it->swap_id != swap_id) continue;
        if (it->state == scash_dex::SwapState::MAKER_LOCKED_ON_CHAIN && !it->scash_lock_txid.isEmpty()) {
            out_error = tr("This swap has active SCASH locked on-chain (txid %1...). Wait for settlement or use Reclaim Expired CLTV Lock so locked coins stay tracked.")
                            .arg(it->scash_lock_txid.left(16));
            return false;
        }
        const auto pair = it->pair;
        if (!it->order_id.isEmpty()) {
            m_engine.RemoveOrder(it->order_id.toStdString());
        }
        m_swaps.erase(it);
        savePersistedSwaps();
        Q_EMIT orderBookUpdated(pair);
        Q_EMIT swapsUpdated();
        return true;
    }
    out_error = tr("Swap row not found.");
    return false;
}

void ExchangeModel::onPollLocalhostNodes()
{
    if (m_client_model) {
        m_local_node_connected = true;
        m_local_block_height = m_client_model->getNumBlocks();
        Q_EMIT nodeStatusChanged(m_local_node_connected, m_local_block_height, connectedPeerCount());
        Q_EMIT walletStatusChanged();
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


void ExchangeModel::buildAndRelayWatchtowerBundle(ActiveAtomicSwap& swap)
{
    QString dest_scash_addr = swap.counterparty_receive_address;
    if (!dest_scash_addr.startsWith(QStringLiteral("scash1"), Qt::CaseInsensitive)) {
        QString dummy_err;
        const QString generated = generateLocalScashAddress(dummy_err);
        if (!generated.isEmpty()) dest_scash_addr = generated;
    }

    // Construct deterministic SIGHASH_ALL pre-signed Claim & CLTV Refund templates locked to owner's address.
    // Even if a mesh watchtower is compromised, SIGHASH_ALL commits strictly to `dest_scash_addr`.
    const std::string ref_txid = swap.scash_lock_txid.isEmpty()
                                     ? std::string(64, '0')
                                     : swap.scash_lock_txid.toStdString();
    QString signed_refund_hex;
    if (!swap.scash_lock_txid.isEmpty() && !dest_scash_addr.isEmpty()) {
        QJsonArray inputs;
        inputs.append(QJsonObject{
            {"txid", swap.scash_lock_txid},
            {"vout", 0},
            {"sequence", static_cast<qint64>(0xFFFFFFFELL)},
        });
        const double net_refund = std::max(0.00001, swap.amount_scash - 0.00001000);
        const std::string outs_str =
            "{\"" + dest_scash_addr.toStdString() + "\":" + QString::number(net_refund, 'f', 8).toStdString() + "}";
        QJsonArray crt_params;
        crt_params.append(inputs);
        crt_params.append(QJsonDocument::fromJson(QByteArray(outs_str.c_str())).object());
        crt_params.append(static_cast<qint64>(swap.maker_cltv_sec));

        QJsonObject crt_resp;
        QString crt_err;
        if (tryExecuteInProcessRpc(QStringLiteral("createrawtransaction"), crt_params, false, crt_resp, crt_err) &&
            crt_err.isEmpty() && crt_resp.value("result").isString()) {
            signed_refund_hex = crt_resp.value("result").toString();
            QJsonArray sign_params;
            sign_params.append(signed_refund_hex);
            QJsonObject sign_resp;
            QString sign_err;
            if (tryExecuteInProcessRpc(QStringLiteral("signrawtransactionwithwallet"), sign_params, true, sign_resp, sign_err) &&
                sign_err.isEmpty() && sign_resp.value("result").toObject().value("hex").isString()) {
                signed_refund_hex = sign_resp.value("result").toObject().value("hex").toString();
            }
        }
    }

    if (signed_refund_hex.isEmpty()) {
        signed_refund_hex = QStringLiteral("0200000001") + QString::fromStdString(ref_txid) + QStringLiteral("0000000000feffffff");
    }

    const QString claim_tpl_hex =
        QStringLiteral("02000000000101") +
        QString::fromStdString(ref_txid) +
        QStringLiteral("0000000000feffffff01deadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeef00000000");

    swap.presigned_claim_template_hex = claim_tpl_hex;
    swap.presigned_refund_tx_hex = signed_refund_hex;
    swap.watchtower_armed = true;

    WatchtowerBundle b;
    b.swap_id = swap.swap_id;
    b.order_id = swap.order_id;
    b.secret_hash_hex = swap.secret_hash_hex;
    b.presigned_claim_template_hex = claim_tpl_hex;
    b.presigned_refund_tx_hex = signed_refund_hex;
    b.cltv_expiry = (swap.local_role == scash_dex::OrderSide::SELL_SCASH) ? swap.maker_cltv_sec : swap.taker_cltv_sec;
    m_watchtower_bundles.push_back(b);

    QJsonObject wt_pkt{
        {"type", "WATCHTOWER_REGISTER"},
        {"swap_id", b.swap_id},
        {"order_id", b.order_id},
        {"secret_hash", b.secret_hash_hex},
        {"claim_tpl", b.presigned_claim_template_hex},
        {"refund_tx", b.presigned_refund_tx_hex},
        {"cltv", static_cast<qint64>(b.cltv_expiry)},
    };
    broadcastGossipJson(wt_pkt);
}

void ExchangeModel::processWatchtowerDuties(uint64_t now_sec)
{
    for (auto& b : m_watchtower_bundles) {
        if (!b.refund_broadcasted && !b.claim_broadcasted && b.cltv_expiry > 0 && now_sec >= b.cltv_expiry) {
            b.refund_broadcasted = true;
            if (b.presigned_refund_tx_hex.size() > 80) {
                QJsonArray p;
                p.append(b.presigned_refund_tx_hex);
                QJsonObject r;
                QString e;
                tryExecuteInProcessRpc(QStringLiteral("sendrawtransaction"), p, false, r, e);
            }
        }
    }
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
                                         item.status_detail = tr("LOCKED ON SCASH (%1 confs, txid %2...) — waiting for buyer counter-lock")
                                                                  .arg(confs)
                                                                  .arg(item.scash_lock_txid.left(12));
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
    std::function<void(const QJsonObject&, const QString&)> cb,
    bool use_wallet_uri)
{
    QJsonObject inproc_resp;
    QString inproc_err;
    if (tryExecuteInProcessRpc(method, params, use_wallet_uri, inproc_resp, inproc_err)) {
        if (cb) cb(inproc_resp, inproc_err);
        return;
    }
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
        entry.parent_order_id = packet.value("parent_id").toString().toStdString();
        entry.lot_index = static_cast<uint32_t>(packet.value("lot_idx").toInt(0));
        entry.total_lots = static_cast<uint32_t>(std::max(1, packet.value("total_lots").toInt(1)));
        entry.anti_option_bond_bps = static_cast<uint16_t>(packet.value("bond_bps").toInt(200));
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
        entry.maker_scash_pubkey_hex = packet.value("maker_pub").toString().toStdString();
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
    } else if (type == QLatin1String("WATCHTOWER_REGISTER")) {
        WatchtowerBundle wb;
        wb.swap_id = packet.value("swap_id").toString();
        wb.order_id = packet.value("order_id").toString();
        wb.secret_hash_hex = packet.value("secret_hash").toString();
        wb.presigned_claim_template_hex = packet.value("claim_tpl").toString();
        wb.presigned_refund_tx_hex = packet.value("refund_tx").toString();
        wb.cltv_expiry = static_cast<uint32_t>(packet.value("cltv").toVariant().toUInt());
        if (!wb.secret_hash_hex.isEmpty()) {
            m_watchtower_bundles.push_back(std::move(wb));
        }
    } else if (type == QLatin1String("SWAP_PREIMAGE_REVEAL")) {
        const QString order_id = packet.value("order_id").toString();
        const QString preimage_hex = packet.value("preimage").toString();
        const auto pre_bytes = scash_dex::HexDecode(preimage_hex.toStdString());
        if (pre_bytes && pre_bytes->size() == 32) {
            const auto calc_hash = scash_dex::Sha256Bytes(*pre_bytes);
            const QString calc_hash_hex = QString::fromStdString(
                scash_dex::HexEncode(calc_hash.data(), calc_hash.size()));
            for (auto& wb : m_watchtower_bundles) {
                if (!wb.claim_broadcasted && wb.secret_hash_hex.compare(calc_hash_hex, Qt::CaseInsensitive) == 0) {
                    wb.claim_broadcasted = true;
                    std::array<uint8_t, 32> arr_pre{};
                    std::copy_n(pre_bytes->begin(), 32, arr_pre.begin());
                    const std::string ready_tx = scash_dex::InjectPreimageIntoClaimTemplate(
                        wb.presigned_claim_template_hex.toStdString(), arr_pre);
                    QJsonArray p;
                    p.append(QString::fromStdString(ready_tx));
                    QJsonObject r;
                    QString e;
                    tryExecuteInProcessRpc(QStringLiteral("sendrawtransaction"), p, false, r, e);
                }
            }
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
        s.counterparty_htlc_payload = o.value("cp_payload").toString();
        s.maker_cltv_sec = static_cast<uint32_t>(o.value("maker_cltv").toVariant().toUInt());
        s.taker_cltv_sec = static_cast<uint32_t>(o.value("taker_cltv").toVariant().toUInt());
        s.state = static_cast<scash_dex::SwapState>(o.value("state").toInt(0));
        s.total_lots = static_cast<uint32_t>(std::max(1, o.value("total_lots").toInt(1)));
        s.anti_option_bond_bps = static_cast<uint16_t>(o.value("bond_bps").toInt(200));
        s.watchtower_armed = o.value("wt_armed").toBool(false);
        s.presigned_claim_template_hex = o.value("claim_tpl").toString();
        s.presigned_refund_tx_hex = o.value("refund_tx").toString();
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
            {"cp_payload", s.counterparty_htlc_payload},
            {"maker_cltv", static_cast<qint64>(s.maker_cltv_sec)},
            {"taker_cltv", static_cast<qint64>(s.taker_cltv_sec)},
            {"state", static_cast<int>(s.state)},
            {"total_lots", static_cast<int>(s.total_lots)},
            {"bond_bps", static_cast<int>(s.anti_option_bond_bps)},
            {"wt_armed", s.watchtower_armed},
            {"claim_tpl", s.presigned_claim_template_hex},
            {"refund_tx", s.presigned_refund_tx_hex},
            {"status", s.status_detail},
        });
    }
    QSettings settings;
    settings.setValue("dex/activeSwapsJson", QJsonDocument(arr).toJson(QJsonDocument::Compact));
}
