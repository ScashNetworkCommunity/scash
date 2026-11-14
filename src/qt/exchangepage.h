// Copyright (c) 2026 The Scash Network Community developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_EXCHANGEPAGE_H
#define BITCOIN_QT_EXCHANGEPAGE_H

#include <qt/exchangehtlc.h>
#include <qt/exchangemodel.h>

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPoint>
#include <QPushButton>
#include <QRect>
#include <QSize>
#include <QString>
#include <QTableWidget>
#include <QWidget>

#include <cstdint>
#include <vector>

class ClientModel;
class PlatformStyle;
class WalletModel;

class CandlestickChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CandlestickChartWidget(QWidget* parent_widget = nullptr);

    void setCandles(std::vector<scash_dex::OhlcvCandle> candles, scash_dex::TradingPair pair);
    void setKawaiiTheme(bool enabled);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    std::vector<scash_dex::OhlcvCandle> m_candles;
    scash_dex::TradingPair m_pair{scash_dex::TradingPair::SCASH_USDT};
    bool m_kawaii{false};
    int m_visible_bars{48};
    QPoint m_hover_pos{-1, -1};
};

class OrderBookDepthWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OrderBookDepthWidget(QWidget* parent_widget = nullptr);

    void setBook(std::vector<scash_dex::AggregatedBookLevel> asks,
                 std::vector<scash_dex::AggregatedBookLevel> bids,
                 scash_dex::TradingPair pair);
    void setKawaiiTheme(bool enabled);

Q_SIGNALS:
    void orderLevelClicked(const QString& order_id, scash_dex::OrderSide maker_side, double price, double amount_scash);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    struct RenderedRow {
        QRect rect;
        QString order_id;
        scash_dex::OrderSide maker_side;
        double price;
        double amount;
    };

    std::vector<scash_dex::AggregatedBookLevel> m_asks;
    std::vector<scash_dex::AggregatedBookLevel> m_bids;
    std::vector<RenderedRow> m_hit_rows;
    scash_dex::TradingPair m_pair{scash_dex::TradingPair::SCASH_USDT};
    bool m_kawaii{false};
    QPoint m_hover_pos{-1, -1};
};

class ExchangePage : public QWidget
{
    Q_OBJECT

public:
    explicit ExchangePage(const PlatformStyle* platform_style, QWidget* parent_widget = nullptr);
    ~ExchangePage() override = default;

    void setClientModel(ClientModel* client_model);
    void setWalletModel(WalletModel* wallet_model);

protected:
    void changeEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private Q_SLOTS:
    void selectPairUsdt();
    void selectPairBtc();
    void selectSideBuy();
    void selectSideSell();
    void onTimeframeChanged(int index);
    void onAutoFillWalletAddress();
    void onShowHowItWorksGuide();
    void onInspectSwapRow(int row);
    void onSubmitLockOrder();
    void onTakeSelectedOrder();
    void onRefundSelectedSwap();
    void onRemoveSelectedSwap();
    void onSaveNodeSettings();
    void onConnectPeerClicked();
    void refreshMarketViews();
    void refreshSwapsTable();
    void updateNodeStatusBanner(bool connected, int block_height, int peer_count);

private:
    bool detectKawaiiTheme() const;
    bool ensurePassphraseReadyForTx();
    void applyAdaptiveTheme();
    void updateAddressFieldPrompts();
    void updateOrderTotalPreview();

    const PlatformStyle* m_platform_style{nullptr};
    ExchangeModel* m_model{nullptr};

    scash_dex::TradingPair m_active_pair{scash_dex::TradingPair::SCASH_USDT};
    scash_dex::OrderSide m_active_side{scash_dex::OrderSide::SELL_SCASH};
    uint32_t m_bucket_seconds{900};
    QString m_selected_order_id;
    QString m_saved_usdt_addr;
    QString m_saved_btc_addr;
    QString m_saved_scash_addr;
    void saveCurrentAddressField();
    void restoreAddressFieldForCurrentMode();
    bool m_kawaii_active{false};
    bool m_applying_theme{false};
    bool m_theme_initialized{false};
    QPixmap m_cached_bg_scaled;
    QSize m_cached_bg_size;

    QFrame* m_header_frame{nullptr};
    QPushButton* m_pair_usdt_btn{nullptr};
    QPushButton* m_pair_btc_btn{nullptr};
    QLabel* m_last_price_label{nullptr};
    QLabel* m_stats_24h_label{nullptr};
    QLabel* m_node_badge_label{nullptr};
    QPushButton* m_how_it_works_btn{nullptr};
    QPushButton* m_settings_toggle_btn{nullptr};

    QFrame* m_settings_drawer{nullptr};
    QLineEdit* m_scash_rpc_edit{nullptr};
    QLineEdit* m_btc_rpc_edit{nullptr};
    QLineEdit* m_usdt_rpc_edit{nullptr};
    QLineEdit* m_usdt_contract_edit{nullptr};
    QComboBox* m_usdt_net_combo{nullptr};
    QLineEdit* m_peer_host_edit{nullptr};

    QFrame* m_chart_frame{nullptr};
    QComboBox* m_timeframe_combo{nullptr};
    CandlestickChartWidget* m_chart_widget{nullptr};
    QFrame* m_book_frame{nullptr};
    OrderBookDepthWidget* m_book_widget{nullptr};

    QFrame* m_ticket_frame{nullptr};
    QPushButton* m_side_sell_btn{nullptr};
    QPushButton* m_side_buy_btn{nullptr};
    QLabel* m_wallet_status_label{nullptr};
    QDoubleSpinBox* m_price_spin{nullptr};
    QDoubleSpinBox* m_amount_spin{nullptr};
    QLabel* m_total_value_label{nullptr};
    QComboBox* m_timelock_combo{nullptr};
    QComboBox* m_lot_split_combo{nullptr};
    QComboBox* m_protection_combo{nullptr};
    QLabel* m_recv_addr_label{nullptr};
    QLineEdit* m_recv_addr_edit{nullptr};
    QPushButton* m_autofill_wallet_btn{nullptr};
    QLabel* m_selected_order_hint{nullptr};
    QPushButton* m_lock_submit_btn{nullptr};
    QPushButton* m_take_order_btn{nullptr};

    QFrame* m_swaps_frame{nullptr};
    QTableWidget* m_swaps_table{nullptr};
    QPushButton* m_inspect_swap_btn{nullptr};
    QPushButton* m_refund_btn{nullptr};
    QPushButton* m_remove_swap_btn{nullptr};
};

#endif // BITCOIN_QT_EXCHANGEPAGE_H
