// Copyright (c) 2026 The Scash Network Community developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/exchangepage.h>

#include <qt/clientmodel.h>
#include <qt/platformstyle.h>

#include <QAbstractItemView>
#include <QApplication>
#include <QClipboard>
#include <QColor>
#include <QCoreApplication>
#include <QDateTime>
#include <QEvent>
#include <QFont>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QPen>
#include <QPixmap>
#include <QPointF>
#include <QRectF>
#include <QSettings>
#include <QSplitter>
#include <QStringList>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QtGlobal>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <utility>

namespace {

constexpr double kCoinSats = 100000000.0;
constexpr double kUsdtMicro = 1000000.0;

QPoint MouseEventPoint(const QMouseEvent* event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return event->position().toPoint();
#else
    return event->pos();
#endif
}

QString FormatPairPrice(scash_dex::TradingPair pair, double price)
{
    if (pair == scash_dex::TradingPair::SCASH_USDT) {
        return QString::number(price, 'f', 6) + QStringLiteral(" USDT");
    }
    return QString::number(price, 'f', 8) + QStringLiteral(" BTC");
}

QString SwapStateLabel(scash_dex::SwapState st)
{
    switch (st) {
    case scash_dex::SwapState::ORDER_OPEN:
        return QStringLiteral("OPEN ON BOOK");
    case scash_dex::SwapState::MAKER_LOCKED_ON_CHAIN:
        return QStringLiteral("LOCKED ON SCASH");
    case scash_dex::SwapState::TAKER_COUNTER_LOCKED:
        return QStringLiteral("COUNTER-LOCKED");
    case scash_dex::SwapState::PREIMAGE_REVEALED:
        return QStringLiteral("AUTO-CLAIMING");
    case scash_dex::SwapState::SETTLED_ON_CHAIN:
        return QStringLiteral("SETTLED ON-CHAIN");
    case scash_dex::SwapState::REFUNDABLE_EXPIRED:
        return QStringLiteral("CLTV UNLOCKED");
    case scash_dex::SwapState::REFUNDED_ON_CHAIN:
        return QStringLiteral("REFUNDED");
    case scash_dex::SwapState::CANCELLED:
        return QStringLiteral("CANCELLED");
    }
    return QStringLiteral("UNKNOWN");
}

} // namespace

// ============================================================================
// CandlestickChartWidget
// ============================================================================

CandlestickChartWidget::CandlestickChartWidget(QWidget* parent_widget)
    : QWidget(parent_widget)
{
    setMinimumSize(360, 250);
    setMouseTracking(true);
}

void CandlestickChartWidget::setCandles(std::vector<scash_dex::OhlcvCandle> candles,
                                        scash_dex::TradingPair pair)
{
    m_candles = std::move(candles);
    m_pair = pair;
    update();
}

void CandlestickChartWidget::setKawaiiTheme(bool enabled)
{
    if (m_kawaii != enabled) {
        m_kawaii = enabled;
        update();
    }
}

void CandlestickChartWidget::mouseMoveEvent(QMouseEvent* event)
{
    m_hover_pos = MouseEventPoint(event);
    update();
}

void CandlestickChartWidget::leaveEvent(QEvent*)
{
    m_hover_pos = QPoint(-1, -1);
    update();
}

void CandlestickChartWidget::wheelEvent(QWheelEvent* event)
{
    const int delta = event->angleDelta().y();
    if (delta > 0) {
        m_visible_bars = std::max(16, m_visible_bars - 6);
    } else if (delta < 0) {
        m_visible_bars = std::min(120, m_visible_bars + 6);
    }
    update();
    event->accept();
}

void CandlestickChartWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const QColor bg = m_kawaii ? QColor("#130f20") : palette().color(QPalette::Base);
    const QColor grid_col = m_kawaii ? QColor(79, 209, 197, 38) : palette().color(QPalette::Mid).lighter(120);
    const QColor text_muted = m_kawaii ? QColor("#e9d5ff") : palette().color(QPalette::PlaceholderText);
    const QColor bull_col = m_kawaii ? QColor("#6ee7b7") : QColor("#16a34a");
    const QColor bear_col = m_kawaii ? QColor("#fb7185") : QColor("#dc2626");
    const QColor ema9_col = m_kawaii ? QColor("#5eead4") : QColor("#2563eb");
    const QColor ema21_col = m_kawaii ? QColor("#fde047") : QColor("#d97706");

    QPainterPath clip;
    clip.addRoundedRect(rect(), 10, 10);
    p.fillPath(clip, bg);

    const int right_axis_w = 88;
    const int bottom_axis_h = 24;
    const QRect plot_rect(12, 12, std::max(40, width() - right_axis_w - 20),
                          std::max(40, height() - bottom_axis_h - 20));
    const int price_h = static_cast<int>(plot_rect.height() * 0.78);
    const QRect price_rect(plot_rect.left(), plot_rect.top(), plot_rect.width(), price_h);
    const QRect vol_rect(plot_rect.left(), price_rect.bottom() + 6,
                         plot_rect.width(), plot_rect.height() - price_h - 6);

    p.setPen(QPen(grid_col, 1, Qt::DashLine));
    for (int i = 0; i <= 4; ++i) {
        const int y = price_rect.top() + (price_rect.height() * i) / 4;
        p.drawLine(price_rect.left(), y, price_rect.right(), y);
    }

    if (m_candles.empty()) {
        p.setPen(text_muted);
        QFont f = font();
        f.setPointSize(10);
        p.setFont(f);
        p.drawText(plot_rect, Qt::AlignCenter,
                   tr("Awaiting on-chain HTLC settlements on localhost Scash node...\n"
                      "Every verified atomic swap prints live OHLCV candles here automatically."));
        return;
    }

    const size_t count = std::min(m_candles.size(), static_cast<size_t>(m_visible_bars));
    const size_t start_idx = m_candles.size() - count;

    double min_p = m_candles[start_idx].low;
    double max_p = m_candles[start_idx].high;
    double max_vol = 0.0001;
    for (size_t i = start_idx; i < m_candles.size(); ++i) {
        min_p = std::min(min_p, m_candles[i].low);
        max_p = std::max(max_p, m_candles[i].high);
        max_vol = std::max(max_vol, m_candles[i].volume_scash);
    }
    const double pad = (max_p - min_p) > 0.0 ? (max_p - min_p) * 0.08 : max_p * 0.05 + 0.000001;
    min_p = std::max(0.0, min_p - pad);
    max_p += pad;

    auto priceToY = [&](double val) -> double {
        const double norm = (val - min_p) / std::max(1e-12, max_p - min_p);
        return price_rect.bottom() - norm * price_rect.height();
    };

    p.setPen(text_muted);
    QFont small_f = font();
    small_f.setPointSize(8);
    p.setFont(small_f);
    const int decimals = (m_pair == scash_dex::TradingPair::SCASH_USDT) ? 5 : 8;
    for (int i = 0; i <= 4; ++i) {
        const double val = max_p - ((max_p - min_p) * i) / 4.0;
        const int y = price_rect.top() + (price_rect.height() * i) / 4;
        p.drawText(QRect(price_rect.right() + 6, y - 8, right_axis_w - 8, 16),
                   Qt::AlignLeft | Qt::AlignVCenter, QString::number(val, 'f', decimals));
    }

    const double step_x = static_cast<double>(price_rect.width()) / static_cast<double>(count);
    const double body_w = std::clamp(step_x * 0.64, 3.0, 18.0);

    QPainterPath ema9_path;
    QPainterPath ema21_path;
    double ema9 = m_candles[start_idx].close;
    double ema21 = m_candles[start_idx].close;
    const double k9 = 2.0 / 10.0;
    const double k21 = 2.0 / 22.0;

    int hovered_index = -1;

    for (size_t i = 0; i < count; ++i) {
        const auto& bar = m_candles[start_idx + i];
        const double cx = price_rect.left() + (i + 0.5) * step_x;
        const double y_open = priceToY(bar.open);
        const double y_close = priceToY(bar.close);
        const double y_high = priceToY(bar.high);
        const double y_low = priceToY(bar.low);
        const bool bull = (bar.close >= bar.open);
        const QColor col = bull ? bull_col : bear_col;

        p.setPen(QPen(col, 1.3));
        p.drawLine(QPointF(cx, y_high), QPointF(cx, y_low));

        const double top_y = std::min(y_open, y_close);
        const double bh = std::max(2.0, std::abs(y_close - y_open));
        p.fillRect(QRectF(cx - body_w * 0.5, top_y, body_w, bh), col);

        const double vh = (bar.volume_scash / max_vol) * vol_rect.height();
        QColor vcol = col;
        vcol.setAlpha(115);
        p.fillRect(QRectF(cx - body_w * 0.5, vol_rect.bottom() - vh, body_w, vh), vcol);

        ema9 = bar.close * k9 + ema9 * (1.0 - k9);
        ema21 = bar.close * k21 + ema21 * (1.0 - k21);
        const QPointF pt9(cx, priceToY(ema9));
        const QPointF pt21(cx, priceToY(ema21));
        if (i == 0) {
            ema9_path.moveTo(pt9);
            ema21_path.moveTo(pt21);
        } else {
            ema9_path.lineTo(pt9);
            ema21_path.lineTo(pt21);
        }

        if (m_hover_pos.x() >= cx - step_x * 0.5 && m_hover_pos.x() < cx + step_x * 0.5) {
            hovered_index = static_cast<int>(start_idx + i);
        }
    }

    p.setPen(QPen(ema9_col, 1.4));
    p.drawPath(ema9_path);
    p.setPen(QPen(ema21_col, 1.4));
    p.drawPath(ema21_path);

    if (hovered_index >= 0 && plot_rect.contains(m_hover_pos)) {
        p.setPen(QPen(text_muted, 1, Qt::DotLine));
        p.drawLine(plot_rect.left(), m_hover_pos.y(), plot_rect.right(), m_hover_pos.y());
        p.drawLine(m_hover_pos.x(), plot_rect.top(), m_hover_pos.x(), plot_rect.bottom());

        const auto& hc = m_candles[static_cast<size_t>(hovered_index)];
        const QString ohlc_line =
            QStringLiteral("%1  O:%2  H:%3  L:%4  C:%5  Vol:%6 SCASH")
                .arg(QDateTime::fromSecsSinceEpoch(static_cast<qint64>(hc.bucket_start_sec))
                         .toString("MM-dd HH:mm"),
                     QString::number(hc.open, 'f', decimals),
                     QString::number(hc.high, 'f', decimals),
                     QString::number(hc.low, 'f', decimals),
                     QString::number(hc.close, 'f', decimals),
                     QString::number(hc.volume_scash, 'f', 2));
        p.setPen(m_kawaii ? QColor("#f3e8ff") : palette().color(QPalette::Text));
        p.drawText(QRect(price_rect.left() + 6, price_rect.top() + 2, price_rect.width() - 12, 18),
                   Qt::AlignLeft | Qt::AlignVCenter, ohlc_line);
    }
}

// ============================================================================
// OrderBookDepthWidget
// ============================================================================

OrderBookDepthWidget::OrderBookDepthWidget(QWidget* parent_widget)
    : QWidget(parent_widget)
{
    setMinimumSize(250, 260);
    setMouseTracking(true);
    setCursor(Qt::PointingHandCursor);
}

void OrderBookDepthWidget::setBook(std::vector<scash_dex::OrderBookEntry> asks,
                                   std::vector<scash_dex::OrderBookEntry> bids,
                                   scash_dex::TradingPair pair)
{
    m_asks = std::move(asks);
    m_bids = std::move(bids);
    m_pair = pair;
    update();
}

void OrderBookDepthWidget::setKawaiiTheme(bool enabled)
{
    if (m_kawaii != enabled) {
        m_kawaii = enabled;
        update();
    }
}

void OrderBookDepthWidget::mouseMoveEvent(QMouseEvent* event)
{
    m_hover_pos = MouseEventPoint(event);
    update();
}

void OrderBookDepthWidget::leaveEvent(QEvent*)
{
    m_hover_pos = QPoint(-1, -1);
    update();
}

void OrderBookDepthWidget::mousePressEvent(QMouseEvent* event)
{
    const QPoint pt = MouseEventPoint(event);
    for (const auto& row : m_hit_rows) {
        if (row.rect.contains(pt)) {
            Q_EMIT orderLevelClicked(row.order_id, row.maker_side, row.price, row.amount);
            return;
        }
    }
}

void OrderBookDepthWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    m_hit_rows.clear();

    const QColor bg = m_kawaii ? QColor("#130f20") : palette().color(QPalette::Base);
    const QColor fg = m_kawaii ? QColor("#f3e8ff") : palette().color(QPalette::Text);
    const QColor muted = m_kawaii ? QColor("#e9d5ff") : palette().color(QPalette::PlaceholderText);
    const QColor ask_col = m_kawaii ? QColor("#fb7185") : QColor("#dc2626");
    const QColor bid_col = m_kawaii ? QColor("#6ee7b7") : QColor("#16a34a");
    const QColor spread_bg = m_kawaii ? QColor("#271e42") : palette().color(QPalette::AlternateBase);

    QPainterPath clip;
    clip.addRoundedRect(rect(), 10, 10);
    p.fillPath(clip, bg);

    const int decimals = (m_pair == scash_dex::TradingPair::SCASH_USDT) ? 6 : 8;
    const double quote_div = (m_pair == scash_dex::TradingPair::SCASH_USDT) ? kUsdtMicro : kCoinSats;

    QFont hdr_font = font();
    hdr_font.setPointSize(8);
    hdr_font.setBold(true);
    p.setFont(hdr_font);
    p.setPen(muted);

    const int w = width();
    p.drawText(QRect(8, 4, w / 3, 18), Qt::AlignLeft | Qt::AlignVCenter,
               m_pair == scash_dex::TradingPair::SCASH_USDT ? tr("Price (USDT)") : tr("Price (BTC)"));
    p.drawText(QRect(w / 3, 4, w / 3, 18), Qt::AlignRight | Qt::AlignVCenter, tr("SCASH"));
    p.drawText(QRect((2 * w) / 3, 4, w / 3 - 8, 18), Qt::AlignRight | Qt::AlignVCenter, tr("On-Chain"));

    const int top_y = 24;
    const int spread_h = 26;
    const int half_h = std::max(40, (height() - top_y - spread_h - 6) / 2);
    const int row_h = 20;
    const size_t max_rows = static_cast<size_t>(std::max(1, half_h / row_h));

    double max_cum_sats = 1.0;
    double cum_a = 0.0;
    for (size_t i = 0; i < std::min(m_asks.size(), max_rows); ++i) {
        cum_a += static_cast<double>(m_asks[i].amount_scash_sats);
        max_cum_sats = std::max(max_cum_sats, cum_a);
    }
    double cum_b = 0.0;
    for (size_t i = 0; i < std::min(m_bids.size(), max_rows); ++i) {
        cum_b += static_cast<double>(m_bids[i].amount_scash_sats);
        max_cum_sats = std::max(max_cum_sats, cum_b);
    }

    QFont row_font = font();
    row_font.setPointSize(8);
    p.setFont(row_font);

    const size_t ask_cnt = std::min(m_asks.size(), max_rows);
    double running_ask = 0.0;
    for (size_t i = 0; i < ask_cnt; ++i) {
        const auto& a = m_asks[i];
        running_ask += static_cast<double>(a.amount_scash_sats);
        const int y = top_y + half_h - static_cast<int>((i + 1) * row_h);
        const QRect r(4, y, w - 8, row_h - 1);

        QColor bar_c = ask_col;
        bar_c.setAlpha(r.contains(m_hover_pos) ? 75 : 40);
        const int bar_w = static_cast<int>((running_ask / max_cum_sats) * r.width());
        p.fillRect(QRect(r.right() - bar_w, r.top(), bar_w, r.height()), bar_c);

        const double pr = static_cast<double>(a.price_quote_units) / quote_div;
        const double amt = static_cast<double>(a.amount_scash_sats) / kCoinSats;

        p.setPen(ask_col);
        p.drawText(QRect(8, y, w / 3, row_h), Qt::AlignLeft | Qt::AlignVCenter,
                   QString::number(pr, 'f', decimals));
        p.setPen(fg);
        p.drawText(QRect(w / 3, y, w / 3, row_h), Qt::AlignRight | Qt::AlignVCenter,
                   QString::number(amt, 'f', 2));
        p.setPen(a.utxo_verified_on_localhost ? bid_col : muted);
        p.drawText(QRect((2 * w) / 3, y, w / 3 - 8, row_h), Qt::AlignRight | Qt::AlignVCenter,
                   a.utxo_verified_on_localhost ? QStringLiteral("UTXO OK") : QStringLiteral("P2P"));

        m_hit_rows.push_back(RenderedRow{
            r, QString::fromStdString(a.order_id), scash_dex::OrderSide::SELL_SCASH, pr, amt});
    }

    const QRect spread_rect(4, top_y + half_h, w - 8, spread_h);
    p.fillRect(spread_rect, spread_bg);
    QString spread_txt = tr("Click any order to fill atomically");
    if (!m_asks.empty() && !m_bids.empty()) {
        const double best_ask = static_cast<double>(m_asks.front().price_quote_units) / quote_div;
        const double best_bid = static_cast<double>(m_bids.front().price_quote_units) / quote_div;
        const double sp = std::max(0.0, best_ask - best_bid);
        spread_txt = tr("Spread: %1").arg(QString::number(sp, 'f', decimals));
    }
    p.setPen(m_kawaii ? QColor("#5eead4") : fg);
    p.drawText(spread_rect, Qt::AlignCenter, spread_txt);

    const size_t bid_cnt = std::min(m_bids.size(), max_rows);
    double running_bid = 0.0;
    const int bids_start_y = spread_rect.bottom() + 2;
    for (size_t i = 0; i < bid_cnt; ++i) {
        const auto& b = m_bids[i];
        running_bid += static_cast<double>(b.amount_scash_sats);
        const int y = bids_start_y + static_cast<int>(i * row_h);
        const QRect r(4, y, w - 8, row_h - 1);

        QColor bar_c = bid_col;
        bar_c.setAlpha(r.contains(m_hover_pos) ? 75 : 40);
        const int bar_w = static_cast<int>((running_bid / max_cum_sats) * r.width());
        p.fillRect(QRect(r.right() - bar_w, r.top(), bar_w, r.height()), bar_c);

        const double pr = static_cast<double>(b.price_quote_units) / quote_div;
        const double amt = static_cast<double>(b.amount_scash_sats) / kCoinSats;

        p.setPen(bid_col);
        p.drawText(QRect(8, y, w / 3, row_h), Qt::AlignLeft | Qt::AlignVCenter,
                   QString::number(pr, 'f', decimals));
        p.setPen(fg);
        p.drawText(QRect(w / 3, y, w / 3, row_h), Qt::AlignRight | Qt::AlignVCenter,
                   QString::number(amt, 'f', 2));
        p.setPen(muted);
        p.drawText(QRect((2 * w) / 3, y, w / 3 - 8, row_h), Qt::AlignRight | Qt::AlignVCenter,
                   QStringLiteral("HTLC"));

        m_hit_rows.push_back(RenderedRow{
            r, QString::fromStdString(b.order_id), scash_dex::OrderSide::BUY_SCASH, pr, amt});
    }
}

// ============================================================================
// ExchangePage
// ============================================================================

ExchangePage::ExchangePage(const PlatformStyle* platform_style, QWidget* parent_widget)
    : QWidget(parent_widget),
      m_platform_style(platform_style),
      m_model(new ExchangeModel(this))
{
    auto* root_layout = new QVBoxLayout(this);
    root_layout->setContentsMargins(12, 10, 12, 10);
    root_layout->setSpacing(8);

    m_header_frame = new QFrame(this);
    m_header_frame->setObjectName("exchangeHeaderFrame");
    auto* hdr_layout = new QHBoxLayout(m_header_frame);
    hdr_layout->setContentsMargins(12, 8, 12, 8);
    hdr_layout->setSpacing(10);

    m_pair_usdt_btn = new QPushButton(tr("SCASH / USDT"), m_header_frame);
    m_pair_usdt_btn->setCheckable(true);
    m_pair_usdt_btn->setChecked(true);

    m_pair_btc_btn = new QPushButton(tr("SCASH / BTC"), m_header_frame);
    m_pair_btc_btn->setCheckable(true);

    m_last_price_label = new QLabel(tr("-- USDT"), m_header_frame);
    QFont price_f = font();
    price_f.setPointSize(13);
    price_f.setBold(true);
    m_last_price_label->setFont(price_f);

    m_stats_24h_label = new QLabel(tr("24h Chg: 0.00%   High: --   Low: --   Vol: 0.00 SCASH"), m_header_frame);
    m_node_badge_label = new QLabel(tr("Localhost Node: Connecting 127.0.0.1:8332..."), m_header_frame);

    m_settings_toggle_btn = new QPushButton(tr("Node & Mesh Config"), m_header_frame);
    m_settings_toggle_btn->setCheckable(true);
    if (m_platform_style) {
        m_settings_toggle_btn->setIcon(m_platform_style->SingleColorIcon(":/icons/options"));
    }

    hdr_layout->addWidget(m_pair_usdt_btn);
    hdr_layout->addWidget(m_pair_btc_btn);
    hdr_layout->addSpacing(8);
    hdr_layout->addWidget(m_last_price_label);
    hdr_layout->addWidget(m_stats_24h_label, 1);
    hdr_layout->addWidget(m_node_badge_label);
    hdr_layout->addWidget(m_settings_toggle_btn);
    root_layout->addWidget(m_header_frame);

    m_settings_drawer = new QFrame(this);
    m_settings_drawer->setObjectName("exchangeSettingsDrawer");
    m_settings_drawer->setVisible(false);
    auto* cfg_grid = new QGridLayout(m_settings_drawer);
    cfg_grid->setContentsMargins(12, 8, 12, 8);
    cfg_grid->setHorizontalSpacing(10);
    cfg_grid->setVerticalSpacing(6);

    m_scash_rpc_edit = new QLineEdit(m_model->scashRpcUrl(), m_settings_drawer);
    m_btc_rpc_edit = new QLineEdit(m_model->btcRpcUrl(), m_settings_drawer);
    m_usdt_rpc_edit = new QLineEdit(m_model->usdtRpcUrl(), m_settings_drawer);
    m_usdt_contract_edit = new QLineEdit(m_model->usdtHtlcContract(), m_settings_drawer);

    m_usdt_net_combo = new QComboBox(m_settings_drawer);
    m_usdt_net_combo->addItem(tr("Ethereum Mainnet (ERC-20 USDT)"), static_cast<int>(scash_dex::UsdtNetwork::ERC20_ETHEREUM));
    m_usdt_net_combo->addItem(tr("Arbitrum One (ERC-20 USDT)"), static_cast<int>(scash_dex::UsdtNetwork::ERC20_ARBITRUM));
    m_usdt_net_combo->addItem(tr("Tron Mainnet (TRC-20 USDT)"), static_cast<int>(scash_dex::UsdtNetwork::TRC20_TRON));

    m_peer_host_edit = new QLineEdit(m_settings_drawer);
    m_peer_host_edit->setPlaceholderText(tr("peer-ip-or-domain:8345"));
    auto* add_peer_btn = new QPushButton(tr("Connect Peer"), m_settings_drawer);
    auto* save_cfg_btn = new QPushButton(tr("Save Endpoints"), m_settings_drawer);

    cfg_grid->addWidget(new QLabel(tr("Local Scash RPC:"), m_settings_drawer), 0, 0);
    cfg_grid->addWidget(m_scash_rpc_edit, 0, 1);
    cfg_grid->addWidget(new QLabel(tr("Native BTC RPC:"), m_settings_drawer), 0, 2);
    cfg_grid->addWidget(m_btc_rpc_edit, 0, 3);
    cfg_grid->addWidget(save_cfg_btn, 0, 4);

    cfg_grid->addWidget(new QLabel(tr("USDT Network:"), m_settings_drawer), 1, 0);
    cfg_grid->addWidget(m_usdt_net_combo, 1, 1);
    cfg_grid->addWidget(new QLabel(tr("USDT RPC URL:"), m_settings_drawer), 1, 2);
    cfg_grid->addWidget(m_usdt_rpc_edit, 1, 3);

    cfg_grid->addWidget(new QLabel(tr("USDT HTLC Contract:"), m_settings_drawer), 2, 0);
    cfg_grid->addWidget(m_usdt_contract_edit, 2, 1);
    cfg_grid->addWidget(new QLabel(tr("Add DEX Mesh Peer:"), m_settings_drawer), 2, 2);
    cfg_grid->addWidget(m_peer_host_edit, 2, 3);
    cfg_grid->addWidget(add_peer_btn, 2, 4);

    root_layout->addWidget(m_settings_drawer);

    auto* main_splitter = new QSplitter(Qt::Vertical, this);

    auto* top_workspace = new QWidget(main_splitter);
    auto* ws_layout = new QHBoxLayout(top_workspace);
    ws_layout->setContentsMargins(0, 0, 0, 0);
    ws_layout->setSpacing(8);

    m_chart_frame = new QFrame(top_workspace);
    m_chart_frame->setObjectName("exchangeCardFrame");
    auto* chart_vbox = new QVBoxLayout(m_chart_frame);
    chart_vbox->setContentsMargins(10, 8, 10, 8);
    chart_vbox->setSpacing(6);

    auto* chart_bar = new QHBoxLayout();
    auto* chart_title = new QLabel(tr("On-Chain Atomic Swap Chart (EMA 9 / 21)"), m_chart_frame);
    m_timeframe_combo = new QComboBox(m_chart_frame);
    m_timeframe_combo->addItem(tr("1m"), 60);
    m_timeframe_combo->addItem(tr("5m"), 300);
    m_timeframe_combo->addItem(tr("15m"), 900);
    m_timeframe_combo->addItem(tr("1h"), 3600);
    m_timeframe_combo->addItem(tr("4h"), 14400);
    m_timeframe_combo->addItem(tr("1d"), 86400);
    m_timeframe_combo->setCurrentIndex(2);

    chart_bar->addWidget(chart_title, 1);
    chart_bar->addWidget(new QLabel(tr("Interval:"), m_chart_frame));
    chart_bar->addWidget(m_timeframe_combo);
    chart_vbox->addLayout(chart_bar);

    m_chart_widget = new CandlestickChartWidget(m_chart_frame);
    chart_vbox->addWidget(m_chart_widget, 1);
    ws_layout->addWidget(m_chart_frame, 5);

    m_book_frame = new QFrame(top_workspace);
    m_book_frame->setObjectName("exchangeCardFrame");
    auto* book_vbox = new QVBoxLayout(m_book_frame);
    book_vbox->setContentsMargins(10, 8, 10, 8);
    book_vbox->setSpacing(6);
    book_vbox->addWidget(new QLabel(tr("Decentralized P2P Order Book"), m_book_frame));
    m_book_widget = new OrderBookDepthWidget(m_book_frame);
    book_vbox->addWidget(m_book_widget, 1);
    ws_layout->addWidget(m_book_frame, 3);

    m_ticket_frame = new QFrame(top_workspace);
    m_ticket_frame->setObjectName("exchangeCardFrame");
    m_ticket_frame->setMinimumWidth(285);
    auto* ticket_vbox = new QVBoxLayout(m_ticket_frame);
    ticket_vbox->setContentsMargins(12, 10, 12, 10);
    ticket_vbox->setSpacing(8);

    auto* side_row = new QHBoxLayout();
    m_side_sell_btn = new QPushButton(tr("Lock && Sell SCASH"), m_ticket_frame);
    m_side_sell_btn->setCheckable(true);
    m_side_sell_btn->setChecked(true);
    m_side_buy_btn = new QPushButton(tr("Buy SCASH"), m_ticket_frame);
    m_side_buy_btn->setCheckable(true);
    side_row->addWidget(m_side_sell_btn);
    side_row->addWidget(m_side_buy_btn);
    ticket_vbox->addLayout(side_row);

    auto* form = new QFormLayout();
    form->setSpacing(6);

    m_price_spin = new QDoubleSpinBox(m_ticket_frame);
    m_price_spin->setDecimals(6);
    m_price_spin->setRange(0.00000001, 1000000.0);
    m_price_spin->setValue(0.025000);

    m_amount_spin = new QDoubleSpinBox(m_ticket_frame);
    m_amount_spin->setDecimals(4);
    m_amount_spin->setRange(0.0001, 21000000.0);
    m_amount_spin->setSuffix(QStringLiteral(" SCASH"));
    m_amount_spin->setValue(1000.0);

    m_total_value_label = new QLabel(tr("25.000000 USDT"), m_ticket_frame);

    m_timelock_combo = new QComboBox(m_ticket_frame);
    m_timelock_combo->addItem(tr("12 Hours CLTV (Fast)"), 12);
    m_timelock_combo->addItem(tr("24 Hours CLTV (Recommended)"), 24);
    m_timelock_combo->addItem(tr("48 Hours CLTV (Maker Bot)"), 48);
    m_timelock_combo->setCurrentIndex(1);

    m_recv_addr_edit = new QLineEdit(m_ticket_frame);
    m_recv_addr_edit->setPlaceholderText(tr("Your USDT (0x.../T...) or BTC (bc1q...) address"));

    form->addRow(tr("Limit Price:"), m_price_spin);
    form->addRow(tr("Lock Amount:"), m_amount_spin);
    form->addRow(tr("Settlement Total:"), m_total_value_label);
    form->addRow(tr("Safety Timelock:"), m_timelock_combo);
    form->addRow(tr("Payout Address:"), m_recv_addr_edit);
    ticket_vbox->addLayout(form);

    m_selected_order_hint = new QLabel(
        tr("100% Non-Custodial: Coins lock in a SegWit P2WSH HTLC on Scash Core and auto-resolve when satisfied."),
        m_ticket_frame);
    m_selected_order_hint->setWordWrap(true);
    ticket_vbox->addWidget(m_selected_order_hint);

    m_lock_submit_btn = new QPushButton(tr("Lock SCASH On-Chain && Post Order"), m_ticket_frame);
    m_take_order_btn = new QPushButton(tr("Fill Selected Book Order Atomically"), m_ticket_frame);
    m_take_order_btn->setEnabled(false);

    ticket_vbox->addStretch(1);
    ticket_vbox->addWidget(m_lock_submit_btn);
    ticket_vbox->addWidget(m_take_order_btn);
    ws_layout->addWidget(m_ticket_frame, 3);

    main_splitter->addWidget(top_workspace);

    m_swaps_frame = new QFrame(main_splitter);
    m_swaps_frame->setObjectName("exchangeCardFrame");
    auto* swaps_vbox = new QVBoxLayout(m_swaps_frame);
    swaps_vbox->setContentsMargins(10, 8, 10, 8);
    swaps_vbox->setSpacing(6);

    auto* swaps_hdr = new QHBoxLayout();
    swaps_hdr->addWidget(new QLabel(tr("Active On-Chain Locks && Auto-Resolve HTLC Monitor"), m_swaps_frame), 1);
    auto* copy_htlc_btn = new QPushButton(tr("Copy Selected HTLC Address"), m_swaps_frame);
    m_refund_btn = new QPushButton(tr("Reclaim Expired CLTV Lock"), m_swaps_frame);
    swaps_hdr->addWidget(copy_htlc_btn);
    swaps_hdr->addWidget(m_refund_btn);
    swaps_vbox->addLayout(swaps_hdr);

    m_swaps_table = new QTableWidget(0, 8, m_swaps_frame);
    m_swaps_table->setHorizontalHeaderLabels({
        tr("Swap ID"),
        tr("Pair"),
        tr("Side"),
        tr("Price"),
        tr("SCASH Locked"),
        tr("HTLC P2WSH Address"),
        tr("State"),
        tr("On-Chain Watcher Detail"),
    });
    m_swaps_table->horizontalHeader()->setStretchLastSection(true);
    m_swaps_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_swaps_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_swaps_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_swaps_table->verticalHeader()->setVisible(false);
    swaps_vbox->addWidget(m_swaps_table, 1);

    main_splitter->addWidget(m_swaps_frame);
    main_splitter->setStretchFactor(0, 7);
    main_splitter->setStretchFactor(1, 3);
    root_layout->addWidget(main_splitter, 1);

    connect(m_pair_usdt_btn, &QPushButton::clicked, this, &ExchangePage::selectPairUsdt);
    connect(m_pair_btc_btn, &QPushButton::clicked, this, &ExchangePage::selectPairBtc);
    connect(m_side_sell_btn, &QPushButton::clicked, this, &ExchangePage::selectSideSell);
    connect(m_side_buy_btn, &QPushButton::clicked, this, &ExchangePage::selectSideBuy);
    connect(m_timeframe_combo, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &ExchangePage::onTimeframeChanged);
    connect(m_settings_toggle_btn, &QPushButton::toggled, m_settings_drawer, &QWidget::setVisible);
    connect(save_cfg_btn, &QPushButton::clicked, this, &ExchangePage::onSaveNodeSettings);
    connect(add_peer_btn, &QPushButton::clicked, this, &ExchangePage::onConnectPeerClicked);

    connect(m_price_spin, qOverload<double>(&QDoubleSpinBox::valueChanged),
            this, [this](double) { updateOrderTotalPreview(); });
    connect(m_amount_spin, qOverload<double>(&QDoubleSpinBox::valueChanged),
            this, [this](double) { updateOrderTotalPreview(); });

    connect(m_lock_submit_btn, &QPushButton::clicked, this, &ExchangePage::onSubmitLockOrder);
    connect(m_take_order_btn, &QPushButton::clicked, this, &ExchangePage::onTakeSelectedOrder);
    connect(m_refund_btn, &QPushButton::clicked, this, &ExchangePage::onRefundSelectedSwap);

    connect(copy_htlc_btn, &QPushButton::clicked, this, [this]() {
        const int row = m_swaps_table->currentRow();
        if (row >= 0 && m_swaps_table->item(row, 5)) {
            QApplication::clipboard()->setText(m_swaps_table->item(row, 5)->text());
        }
    });

    connect(m_book_widget, &OrderBookDepthWidget::orderLevelClicked, this,
            [this](const QString& order_id, scash_dex::OrderSide maker_side, double price, double amount_scash) {
                m_selected_order_id = order_id;
                m_price_spin->setValue(price);
                m_amount_spin->setValue(amount_scash);
                m_take_order_btn->setEnabled(true);
                m_selected_order_hint->setText(
                    tr("Selected Order %1... (%2 %3 SCASH @ %4) ready for atomic swap.")
                        .arg(order_id.left(10),
                             maker_side == scash_dex::OrderSide::SELL_SCASH ? tr("Buy") : tr("Sell"),
                             QString::number(amount_scash, 'f', 2),
                             FormatPairPrice(m_active_pair, price)));
            });

    connect(m_model, &ExchangeModel::orderBookUpdated, this, [this](scash_dex::TradingPair) {
        refreshMarketViews();
    });
    connect(m_model, &ExchangeModel::candlesUpdated, this, [this](scash_dex::TradingPair) {
        refreshMarketViews();
    });
    connect(m_model, &ExchangeModel::swapsUpdated, this, &ExchangePage::refreshSwapsTable);
    connect(m_model, &ExchangeModel::nodeStatusChanged, this, &ExchangePage::updateNodeStatusBanner);

    applyAdaptiveTheme();
    refreshMarketViews();
    refreshSwapsTable();
}

void ExchangePage::setClientModel(ClientModel* client_model)
{
    m_model->setClientModel(client_model);
}

void ExchangePage::setWalletModel(WalletModel* wallet_model)
{
    m_model->setWalletModel(wallet_model);
}

bool ExchangePage::detectKawaiiTheme() const
{
    if (QCoreApplication::applicationFilePath().contains("kawaii", Qt::CaseInsensitive) ||
        QCoreApplication::applicationName().contains("kawaii", Qt::CaseInsensitive)) {
        return true;
    }
    QSettings settings;
    for (const QString& key : settings.allKeys()) {
        if (key.contains("kawaii", Qt::CaseInsensitive) && settings.value(key).toBool()) {
            return true;
        }
    }
    const QWidget* top = window();
    if (top && top != this && top->styleSheet().contains("#161224", Qt::CaseInsensitive)) {
        return true;
    }
    return false;
}

void ExchangePage::applyAdaptiveTheme()
{
    if (m_applying_theme) {
        return;
    }
    const bool want_kawaii = detectKawaiiTheme();
    if (m_theme_initialized && want_kawaii == m_kawaii_active) {
        return;
    }

    m_applying_theme = true;
    m_kawaii_active = want_kawaii;
    m_theme_initialized = true;

    m_chart_widget->setKawaiiTheme(m_kawaii_active);
    m_book_widget->setKawaiiTheme(m_kawaii_active);

    if (m_kawaii_active) {
        setStyleSheet(QStringLiteral(
            "QFrame#exchangeHeaderFrame, QFrame#exchangeCardFrame, QFrame#exchangeSettingsDrawer {"
            "  background-color: rgba(22, 18, 36, 228);"
            "  border: 2px solid #4fd1c5;"
            "  border-radius: 14px;"
            "}"
            "QLabel { color: #f3e8ff; background: transparent; border: none; }"
            "QLineEdit, QComboBox, QDoubleSpinBox {"
            "  background-color: #1d1730;"
            "  color: #6ee7b7;"
            "  border: 1px solid #4fd1c5;"
            "  border-radius: 7px;"
            "  padding: 4px 7px;"
            "}"
            "QPushButton {"
            "  background-color: #271e42;"
            "  color: #f3e8ff;"
            "  border: 1px solid #a855f7;"
            "  border-radius: 9px;"
            "  padding: 6px 12px;"
            "  font-weight: bold;"
            "}"
            "QPushButton:hover { background-color: #3b2d5e; border-color: #5eead4; }"
            "QPushButton:checked {"
            "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
            "              stop:0 #fb923c, stop:0.5 #ea580c, stop:1 #c2410c);"
            "  border: 2px solid #fde047;"
            "  color: #ffffff;"
            "}"
            "QTableWidget {"
            "  background-color: #161224;"
            "  color: #f3e8ff;"
            "  gridline-color: #3b2d5e;"
            "  border: 1px solid #4fd1c5;"
            "  border-radius: 8px;"
            "  selection-background-color: #9333ea;"
            "  selection-color: #ffffff;"
            "}"
            "QHeaderView::section {"
            "  background-color: #1d1730;"
            "  color: #5eead4;"
            "  border: none;"
            "  border-bottom: 1px solid #4fd1c5;"
            "  padding: 4px;"
            "  font-weight: bold;"
            "}"));
    } else {
        setStyleSheet(QStringLiteral(
            "QFrame#exchangeHeaderFrame, QFrame#exchangeCardFrame, QFrame#exchangeSettingsDrawer {"
            "  border: 1px solid palette(mid);"
            "  border-radius: 8px;"
            "  background-color: palette(window);"
            "}"
            "QPushButton:checked {"
            "  font-weight: bold;"
            "  border: 2px solid palette(highlight);"
            "}"));
    }
    m_applying_theme = false;
    update();
}

void ExchangePage::changeEvent(QEvent* event)
{
    QWidget::changeEvent(event);
    if (!m_applying_theme &&
        (event->type() == QEvent::StyleChange || event->type() == QEvent::PaletteChange || event->type() == QEvent::ParentChange)) {
        applyAdaptiveTheme();
    }
}

void ExchangePage::paintEvent(QPaintEvent* event)
{
    if (m_kawaii_active) {
        QPainter p(this);
        p.fillRect(rect(), QColor("#161224"));
        const QSize cur_size = size();
        if (cur_size.width() > 0 && cur_size.height() > 0) {
            if (m_cached_bg_scaled.isNull() || m_cached_bg_size != cur_size) {
                const QPixmap raw_bg(QStringLiteral(":/icons/scash_kawaii_bg"));
                if (!raw_bg.isNull()) {
                    m_cached_bg_scaled = raw_bg.scaled(cur_size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
                    m_cached_bg_size = cur_size;
                }
            }
            if (!m_cached_bg_scaled.isNull()) {
                p.setOpacity(0.16);
                p.drawPixmap(rect(), m_cached_bg_scaled);
                p.setOpacity(1.0);
            }
        }
    }
    QWidget::paintEvent(event);
}

void ExchangePage::selectPairUsdt()
{
    m_active_pair = scash_dex::TradingPair::SCASH_USDT;
    m_pair_usdt_btn->setChecked(true);
    m_pair_btc_btn->setChecked(false);
    m_price_spin->setDecimals(6);
    m_price_spin->setValue(m_model->lastPrice(m_active_pair) > 0.0 ? m_model->lastPrice(m_active_pair) : 0.025000);
    m_recv_addr_edit->setPlaceholderText(tr("Your ERC-20 (0x...) or TRC-20 (T...) USDT address"));
    m_selected_order_id.clear();
    m_take_order_btn->setEnabled(false);
    refreshMarketViews();
}

void ExchangePage::selectPairBtc()
{
    m_active_pair = scash_dex::TradingPair::SCASH_BTC;
    m_pair_btc_btn->setChecked(true);
    m_pair_usdt_btn->setChecked(false);
    m_price_spin->setDecimals(8);
    m_price_spin->setValue(m_model->lastPrice(m_active_pair) > 0.0 ? m_model->lastPrice(m_active_pair) : 0.00000035);
    m_recv_addr_edit->setPlaceholderText(tr("Your native Bitcoin SegWit address (bc1q...)"));
    m_selected_order_id.clear();
    m_take_order_btn->setEnabled(false);
    refreshMarketViews();
}

void ExchangePage::selectSideSell()
{
    m_active_side = scash_dex::OrderSide::SELL_SCASH;
    m_side_sell_btn->setChecked(true);
    m_side_buy_btn->setChecked(false);
    m_lock_submit_btn->setText(tr("Lock SCASH On-Chain && Post Sell Order"));
}

void ExchangePage::selectSideBuy()
{
    m_active_side = scash_dex::OrderSide::BUY_SCASH;
    m_side_buy_btn->setChecked(true);
    m_side_sell_btn->setChecked(false);
    m_lock_submit_btn->setText(tr("Post Buy Bid && Stage Counter-HTLC"));
}

void ExchangePage::onTimeframeChanged(int index)
{
    m_bucket_seconds = m_timeframe_combo->itemData(index).toUInt();
    if (m_bucket_seconds == 0) m_bucket_seconds = 900;
    refreshMarketViews();
}

void ExchangePage::updateOrderTotalPreview()
{
    const double total = m_price_spin->value() * m_amount_spin->value();
    m_total_value_label->setText(FormatPairPrice(m_active_pair, total));
}

void ExchangePage::onSubmitLockOrder()
{
    QString err;
    const uint32_t lock_hours = m_timelock_combo->currentData().toUInt();
    if (!m_model->createAndLockOrder(m_active_pair,
                                     m_active_side,
                                     m_price_spin->value(),
                                     m_amount_spin->value(),
                                     lock_hours > 0 ? lock_hours : 24,
                                     m_recv_addr_edit->text(),
                                     err)) {
        QMessageBox::warning(this, tr("Cannot Create Atomic Order"), err);
        return;
    }
    refreshMarketViews();
    refreshSwapsTable();
}

void ExchangePage::onTakeSelectedOrder()
{
    if (m_selected_order_id.isEmpty()) return;
    QString err;
    if (!m_model->takeOrderAtomically(m_selected_order_id,
                                      m_amount_spin->value(),
                                      m_recv_addr_edit->text(),
                                      err)) {
        QMessageBox::warning(this, tr("Atomic Fill Failed"), err);
        return;
    }
    refreshMarketViews();
    refreshSwapsTable();
}

void ExchangePage::onRefundSelectedSwap()
{
    const int row = m_swaps_table->currentRow();
    if (row < 0 || !m_swaps_table->item(row, 0)) {
        QMessageBox::information(this, tr("Select Swap"), tr("Select a locked swap row in the table first."));
        return;
    }
    QString err;
    if (!m_model->triggerEmergencyRefund(m_swaps_table->item(row, 0)->text(), err)) {
        QMessageBox::warning(this, tr("CLTV Refund Locked"), err);
    }
}

void ExchangePage::onSaveNodeSettings()
{
    m_model->setScashRpcEndpoint(m_scash_rpc_edit->text(), QString());
    m_model->setBtcRpcEndpoint(m_btc_rpc_edit->text(), QString());
    const auto net = static_cast<scash_dex::UsdtNetwork>(m_usdt_net_combo->currentData().toInt());
    m_model->setUsdtRpcEndpoint(m_usdt_rpc_edit->text(), m_usdt_contract_edit->text(), net);
}

void ExchangePage::onConnectPeerClicked()
{
    const QString raw = m_peer_host_edit->text().trimmed();
    if (raw.isEmpty()) return;
    const QStringList parts = raw.split(':');
    const QString host = parts.value(0);
    const uint16_t port = parts.size() > 1 ? static_cast<uint16_t>(parts.value(1).toUInt()) : 8345;
    m_model->connectToPeer(host, port > 0 ? port : 8345);
    m_peer_host_edit->clear();
}

void ExchangePage::refreshMarketViews()
{
    applyAdaptiveTheme();

    m_book_widget->setBook(m_model->asks(m_active_pair, 18),
                           m_model->bids(m_active_pair, 18),
                           m_active_pair);
    m_chart_widget->setCandles(m_model->candles(m_active_pair, m_bucket_seconds), m_active_pair);

    const double last = m_model->lastPrice(m_active_pair);
    m_last_price_label->setText(last > 0.0 ? FormatPairPrice(m_active_pair, last)
                                           : (m_active_pair == scash_dex::TradingPair::SCASH_USDT
                                                  ? tr("-- USDT")
                                                  : tr("-- BTC")));

    const int dec = (m_active_pair == scash_dex::TradingPair::SCASH_USDT) ? 6 : 8;
    m_stats_24h_label->setText(
        tr("24h Chg: %1%   High: %2   Low: %3   Vol: %4 SCASH")
            .arg(QString::number(m_model->change24hPercent(m_active_pair), 'f', 2),
                 m_model->high24h(m_active_pair) > 0.0 ? QString::number(m_model->high24h(m_active_pair), 'f', dec) : QStringLiteral("--"),
                 m_model->low24h(m_active_pair) > 0.0 ? QString::number(m_model->low24h(m_active_pair), 'f', dec) : QStringLiteral("--"),
                 QString::number(m_model->volume24hScash(m_active_pair), 'f', 2)));

    updateOrderTotalPreview();
}

void ExchangePage::refreshSwapsTable()
{
    const auto& list = m_model->activeSwaps();
    m_swaps_table->setRowCount(static_cast<int>(list.size()));

    for (size_t i = 0; i < list.size(); ++i) {
        const auto& s = list[i];
        const int r = static_cast<int>(i);
        const int dec = (s.pair == scash_dex::TradingPair::SCASH_USDT) ? 6 : 8;

        m_swaps_table->setItem(r, 0, new QTableWidgetItem(s.swap_id));
        m_swaps_table->setItem(r, 1, new QTableWidgetItem(
            s.pair == scash_dex::TradingPair::SCASH_USDT ? QStringLiteral("SCASH/USDT") : QStringLiteral("SCASH/BTC")));
        m_swaps_table->setItem(r, 2, new QTableWidgetItem(
            s.local_role == scash_dex::OrderSide::SELL_SCASH ? tr("SELL SCASH") : tr("BUY SCASH")));
        m_swaps_table->setItem(r, 3, new QTableWidgetItem(QString::number(s.price, 'f', dec)));
        m_swaps_table->setItem(r, 4, new QTableWidgetItem(QString::number(s.amount_scash, 'f', 4)));
        m_swaps_table->setItem(r, 5, new QTableWidgetItem(s.htlc_p2wsh_address));
        m_swaps_table->setItem(r, 6, new QTableWidgetItem(SwapStateLabel(s.state)));
        m_swaps_table->setItem(r, 7, new QTableWidgetItem(s.status_detail));
    }
}

void ExchangePage::updateNodeStatusBanner(bool connected, int block_height, int peer_count)
{
    if (connected) {
        m_node_badge_label->setText(
            tr("Localhost Scash 2.4.0 • Block #%1 • %2 DEX Peers")
                .arg(block_height)
                .arg(peer_count));
    } else {
        m_node_badge_label->setText(
            tr("Localhost RPC Offline (%1) • %2 DEX Peers")
                .arg(m_model->scashRpcUrl())
                .arg(peer_count));
    }
}
