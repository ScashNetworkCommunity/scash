// Copyright (c) 2026 The Scash Network Community developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_EXCHANGEMODEL_H
#define BITCOIN_QT_EXCHANGEMODEL_H

#include <qt/exchangehtlc.h>

#include <QByteArray>
#include <QDateTime>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

class ClientModel;
class WalletModel;

struct ActiveAtomicSwap {
    QString swap_id;
    QString order_id;
    scash_dex::TradingPair pair{scash_dex::TradingPair::SCASH_USDT};
    scash_dex::OrderSide local_role{scash_dex::OrderSide::SELL_SCASH};
    scash_dex::UsdtNetwork usdt_net{scash_dex::UsdtNetwork::ERC20_ETHEREUM};
    double price{0.0};
    double amount_scash{0.0};
    double quote_total{0.0};
    QString htlc_p2wsh_address;
    QString scash_lock_txid;
    QString counterparty_lock_txid;
    QString claim_or_refund_txid;
    QString secret_hash_hex;
    QString secret_preimage_hex;
    QString witness_script_hex;
    QString counterparty_receive_address;
    uint32_t maker_cltv_sec{0};
    uint32_t taker_cltv_sec{0};
    int scash_confirmations{0};
    int counterparty_confirmations{0};
    scash_dex::SwapState state{scash_dex::SwapState::ORDER_OPEN};
    QString status_detail;
};

class ExchangeModel : public QObject
{
    Q_OBJECT

public:
    explicit ExchangeModel(QObject* parent_obj = nullptr);
    ~ExchangeModel() override;

    void setClientModel(ClientModel* client_model);
    void setWalletModel(WalletModel* wallet_model);

    void setScashRpcEndpoint(const QString& url, const QString& cookie_or_userpass);
    void setBtcRpcEndpoint(const QString& url, const QString& userpass);
    void setUsdtRpcEndpoint(const QString& url, const QString& htlc_contract_addr, scash_dex::UsdtNetwork net);

    QString scashRpcUrl() const { return m_scash_rpc_url; }
    QString btcRpcUrl() const { return m_btc_rpc_url; }
    QString usdtRpcUrl() const { return m_usdt_rpc_url; }
    QString usdtHtlcContract() const { return m_usdt_htlc_contract; }
    scash_dex::UsdtNetwork usdtNetwork() const { return m_usdt_network; }

    bool startP2POrderMesh(uint16_t listen_port = 8345);
    void connectToPeer(const QString& host, uint16_t port = 8345);
    int connectedPeerCount() const;
    int localBlockHeight() const { return m_local_block_height; }
    bool isLocalNodeSynced() const { return m_local_node_connected; }
    bool hasLoadedWallet() const { return m_wallet_model != nullptr; }

    std::vector<scash_dex::OrderBookEntry> asks(scash_dex::TradingPair pair, size_t limit = 18) const;
    std::vector<scash_dex::OrderBookEntry> bids(scash_dex::TradingPair pair, size_t limit = 18) const;
    std::vector<scash_dex::OhlcvCandle> candles(scash_dex::TradingPair pair, uint32_t bucket_sec) const;
    const std::vector<ActiveAtomicSwap>& activeSwaps() const { return m_swaps; }

    double lastPrice(scash_dex::TradingPair pair) const;
    double change24hPercent(scash_dex::TradingPair pair) const;
    double high24h(scash_dex::TradingPair pair) const;
    double low24h(scash_dex::TradingPair pair) const;
    double volume24hScash(scash_dex::TradingPair pair) const;

    bool createAndLockOrder(scash_dex::TradingPair pair,
                            scash_dex::OrderSide side,
                            double price,
                            double amount_scash,
                            uint32_t lock_hours,
                            const QString& counterparty_receive_addr,
                            QString& out_error);

    bool takeOrderAtomically(const QString& order_id,
                             double fill_amount_scash,
                             const QString& counterparty_receive_addr,
                             QString& out_error);

    bool triggerEmergencyRefund(const QString& swap_id, QString& out_error);

Q_SIGNALS:
    void orderBookUpdated(scash_dex::TradingPair pair);
    void candlesUpdated(scash_dex::TradingPair pair);
    void swapsUpdated();
    void nodeStatusChanged(bool connected, int block_height, int peer_count);
    void exchangeAlert(const QString& title, const QString& message);

private Q_SLOTS:
    void onPollLocalhostNodes();
    void onNewPeerConnection();
    void onPeerReadyRead();
    void onPeerDisconnected();

private:
    QString autoDetectScashCookieAuth() const;
    void sendScashJsonRpc(const QString& method,
                          const QJsonArray& params,
                          std::function<void(const QJsonObject&, const QString&)> cb);
    void sendExternalJsonRpc(const QString& url,
                             const QString& userpass,
                             const QString& method,
                             const QJsonArray& params,
                             std::function<void(const QJsonObject&, const QString&)> cb);

    void verifyOrderUtxoOnLocalhost(const scash_dex::OrderBookEntry& entry);
    void advanceSwapStateMachine();
    void broadcastGossipJson(const QJsonObject& packet);
    void handleGossipPacket(const QJsonObject& packet, QTcpSocket* sender_peer);
    void loadPersistedSwaps();
    void savePersistedSwaps() const;

    ClientModel* m_client_model{nullptr};
    WalletModel* m_wallet_model{nullptr};

    QTcpServer* m_p2p_server{nullptr};
    QList<QPointer<QTcpSocket>> m_peers;
    std::set<std::string> m_seen_gossip_ids;

    QTimer* m_poll_timer{nullptr};
    QString m_scash_rpc_url{QStringLiteral("http://127.0.0.1:8332")};
    QString m_scash_rpc_auth;
    QString m_btc_rpc_url{QStringLiteral("http://127.0.0.1:8332")};
    QString m_btc_rpc_auth;
    QString m_usdt_rpc_url{QStringLiteral("http://127.0.0.1:8545")};
    QString m_usdt_htlc_contract{QStringLiteral("0x0000000000000000000000000000000000000000")};
    scash_dex::UsdtNetwork m_usdt_network{scash_dex::UsdtNetwork::ERC20_ETHEREUM};

    bool m_local_node_connected{false};
    int m_local_block_height{0};

    scash_dex::OrderBookEngine m_engine;
    std::vector<ActiveAtomicSwap> m_swaps;
    std::map<scash_dex::TradingPair, double> m_last_price;
};

#endif // BITCOIN_QT_EXCHANGEMODEL_H
