// Copyright (c) 2026 The Scash Network Community developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_EXCHANGEHTLC_H
#define BITCOIN_QT_EXCHANGEHTLC_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace scash_dex {

enum class TradingPair : uint8_t {
    SCASH_USDT = 0,
    SCASH_BTC = 1,
};

enum class OrderSide : uint8_t {
    BUY_SCASH = 0,  // Buyer receives SCASH, counter-locks USDT or BTC
    SELL_SCASH = 1, // Seller locks SCASH on-chain, receives USDT or BTC
};

enum class UsdtNetwork : uint8_t {
    ERC20_ETHEREUM = 0, // Official Tether ERC-20 (0xdAC17F958D2ee523a2206206994597C13D831ec7)
    ERC20_ARBITRUM = 1, // Official Tether Arbitrum (0xFd086bC7CD5C481DCC9C85ebE478A1C0b69FCbb9)
    TRC20_TRON = 2,     // Official Tether TRC-20 (TR7NHqjeKQxGTCi8q8ZY4pL8otSzgjLj6t)
};

enum class SwapState : uint8_t {
    ORDER_OPEN = 0,            // Broadcast on P2P mesh
    MAKER_LOCKED_ON_CHAIN = 1, // Maker HTLC locked on Scash chain (scash1q...)
    TAKER_COUNTER_LOCKED = 2,  // Counterparty HTLC staged/verified at exact order price
    PREIMAGE_REVEALED = 3,     // Secret preimage observed on-chain / auto-claiming
    SETTLED_ON_CHAIN = 4,      // Both legs claimed atomically on-chain
    REFUNDABLE_EXPIRED = 5,    // CLTV timelock expired; auto-refund ready
    REFUNDED_ON_CHAIN = 6,     // Locked coins returned to original owner
    CANCELLED = 7,
};

struct HtlcParameters {
    std::array<uint8_t, 32> secret_hash{};           // SHA-256(secret_preimage)
    std::array<uint8_t, 20> recipient_pubkey_hash{}; // HASH160(buyer pubkey)
    std::array<uint8_t, 20> refund_pubkey_hash{};    // HASH160(seller pubkey)
    uint32_t cltv_expiry{0};                         // UNIX timestamp or block height
};

struct OrderBookEntry {
    std::string order_id;          // Deterministic SHA-256 hex of canonical order payload
    std::string parent_order_id;   // Groups multi-lot UTXO outputs into 1 unified UI limit order
    uint32_t lot_index{0};         // 0..N-1 lot index within parent order
    uint32_t total_lots{1};        // Total lots minted in sendmany fan-out
    uint16_t anti_option_bond_bps{200}; // 200 bps (2.00%) anti-free-option griefing bond
    TradingPair pair{TradingPair::SCASH_USDT};
    OrderSide side{OrderSide::SELL_SCASH};
    uint64_t price_quote_units{0}; // Price per 1.00000000 SCASH (in sats for BTC, or 1e-6 USDT units)
    uint64_t amount_scash_sats{0}; // Amount in SCASH satoshis (1 SCASH = 100,000,000 sats)
    std::string utxo_txid;         // On-chain Scash UTXO proving seller liquidity
    uint32_t utxo_vout{0};
    std::string maker_scash_pubkey_hex;
    std::string maker_counterparty_address; // Maker's payout/receive address
    std::string signature_base64;           // Scash wallet signmessage proof
    uint64_t created_at{0};
    uint64_t expires_at{0};
    bool utxo_verified_on_localhost{false};
};

struct OhlcvCandle {
    uint64_t bucket_start_sec{0};
    double open{0.0};
    double high{0.0};
    double low{0.0};
    double close{0.0};
    double volume_scash{0.0};
};

std::array<uint8_t, 32> Sha256Bytes(const uint8_t* data, size_t len);
std::array<uint8_t, 32> Sha256Bytes(const std::vector<uint8_t>& data);
std::array<uint8_t, 20> Hash160Bytes(const std::vector<uint8_t>& data);
std::array<uint8_t, 32> GenerateRandomPreimage();

struct AggregatedBookLevel {
    uint64_t price_quote_units{0};
    uint64_t total_scash_sats{0};
    uint32_t lot_count{0};
    bool all_verified_on_localhost{false};
    std::string representative_order_id;
};

// Deterministic per-lot secret derivation: S_i = SHA256("SCASH-LOT-v2.4|" || master_seed || lot_index_be32)
// Revealing S_i for a partial fill NEVER exposes S_j (j != i) for remaining unfilled lots!
std::array<uint8_t, 32> DeriveIndexedLotPreimage(const std::array<uint8_t, 32>& master_seed, uint32_t lot_index);

// Splits a limit order into up to `max_lots` discrete UTXO lot sizes (exact satoshi sum preserved)
std::vector<uint64_t> SplitAmountIntoLots(uint64_t total_scash_sats,
                                          uint32_t max_lots = 10,
                                          uint64_t min_lot_sats = 10000000ULL);

// Calculates anti-free-option griefing bond in quote/base units (default 200 bps = 2.00%)
uint64_t ComputeAntiOptionBondUnits(uint64_t principal_units, uint16_t bond_bps = 200);

// Splices a revealed 32-byte preimage into a SIGHASH_ALL pre-signed watchtower claim tx template
std::string InjectPreimageIntoClaimTemplate(const std::string& template_hex,
                                            const std::array<uint8_t, 32>& preimage);

std::string HexEncode(const uint8_t* data, size_t len);
std::string HexEncode(const std::vector<uint8_t>& data);
std::optional<std::vector<uint8_t>> HexDecode(const std::string& hex);
bool IsValidCompressedPubKeyHex(const std::string& pubkey_hex);

// Bech32 SegWit v0 P2WSH address encoder ("scash" for Scash mainnet, "bc" for Bitcoin mainnet)
std::string EncodeBech32P2WSH(const std::string& hrp, const std::array<uint8_t, 32>& witness_script_sha256);

std::vector<uint8_t> BuildHtlcWitnessScript(const HtlcParameters& params);
std::vector<uint8_t> BuildP2WSHScriptPubKey(const std::vector<uint8_t>& witness_script);
std::string DeriveHtlcP2WSHAddress(const std::vector<uint8_t>& witness_script, const std::string& hrp = "scash");

bool ValidateAsymmetricTimelocks(uint64_t now_sec,
                                 uint32_t maker_cltv_sec,
                                 uint32_t taker_cltv_sec,
                                 uint32_t min_safety_delta_sec = 21600);

std::vector<std::vector<uint8_t>> BuildHtlcClaimWitnessStack(
    const std::vector<uint8_t>& der_sig_with_sighash,
    const std::vector<uint8_t>& recipient_pubkey,
    const std::array<uint8_t, 32>& secret_preimage,
    const std::vector<uint8_t>& witness_script);

std::vector<std::vector<uint8_t>> BuildHtlcRefundWitnessStack(
    const std::vector<uint8_t>& der_sig_with_sighash,
    const std::vector<uint8_t>& refund_pubkey,
    const std::vector<uint8_t>& witness_script);

std::optional<std::array<uint8_t, 32>> ExtractPreimageFromWitnessItems(
    const std::vector<std::vector<uint8_t>>& witness_items,
    const std::array<uint8_t, 32>& expected_secret_hash);

std::string EncodeErc20HtlcNewContractCalldata(
    const std::string& receiver_evm_address_hex,
    const std::array<uint8_t, 32>& secret_hash,
    uint64_t timelock_unix_sec,
    const std::string& usdt_token_contract_hex,
    uint64_t amount_usdt_micro);

std::string EncodeErc20HtlcWithdrawCalldata(
    const std::array<uint8_t, 32>& contract_id,
    const std::array<uint8_t, 32>& secret_preimage);

std::string EncodeErc20HtlcRefundCalldata(
    const std::array<uint8_t, 32>& contract_id);

std::string BuildCanonicalOrderSigningMessage(const OrderBookEntry& entry);
std::string ComputeOrderId(const OrderBookEntry& entry);

class OrderBookEngine {
public:
    bool UpsertOrder(OrderBookEntry order);
    bool RemoveOrder(const std::string& order_id);
    void MarkUtxoSpent(const std::string& txid, uint32_t vout);

    std::vector<OrderBookEntry> GetAsks(TradingPair pair, size_t limit = 25) const;
    std::vector<OrderBookEntry> GetBids(TradingPair pair, size_t limit = 25) const;
    std::vector<AggregatedBookLevel> GetAggregatedAsks(TradingPair pair, size_t limit = 18) const;
    std::vector<AggregatedBookLevel> GetAggregatedBids(TradingPair pair, size_t limit = 18) const;
    std::vector<OrderBookEntry> MatchTakerOrder(TradingPair pair,
                                                OrderSide taker_side,
                                                uint64_t limit_price_quote_units,
                                                uint64_t max_scash_sats) const;

    void RecordOnChainTrade(TradingPair pair,
                            double price,
                            double volume_scash,
                            uint64_t timestamp_sec);

    std::vector<OhlcvCandle> GetCandles(TradingPair pair,
                                        uint32_t bucket_seconds,
                                        size_t max_candles = 120) const;

private:
    struct TradePrint {
        TradingPair pair;
        double price;
        double volume_scash;
        uint64_t timestamp_sec;
    };

    std::vector<OrderBookEntry> m_orders;
    std::vector<TradePrint> m_trades;
};

} // namespace scash_dex

#endif // BITCOIN_QT_EXCHANGEHTLC_H
