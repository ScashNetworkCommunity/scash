// Copyright (c) 2026 The Scash Network Community developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "exchangehtlc.h"

#include <cassert>
#include <cmath>
#include <iostream>

using namespace scash_dex;

int main()
{
    const std::string abc = "abc";
    const auto sha_abc = Sha256Bytes(reinterpret_cast<const uint8_t*>(abc.data()), abc.size());
    assert(HexEncode(sha_abc.data(), sha_abc.size()) ==
           "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");

    const auto gen_pub = HexDecode("0279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798");
    assert(gen_pub.has_value() && gen_pub->size() == 33);
    const auto gen_pkh = Hash160Bytes(*gen_pub);
    assert(HexEncode(gen_pkh.data(), gen_pkh.size()) == "751e76e8199196d454941c45d1b3a323f1433bd6");

    const auto preimage = GenerateRandomPreimage();
    const auto secret_hash = Sha256Bytes(preimage.data(), preimage.size());

    HtlcParameters params{};
    params.secret_hash = secret_hash;
    params.recipient_pubkey_hash = gen_pkh;
    params.refund_pubkey_hash = gen_pkh;
    params.cltv_expiry = 1795000000U;

    const auto witness_script = BuildHtlcWitnessScript(params);
    assert(!witness_script.empty());
    assert(witness_script.front() == 0x63);
    assert(witness_script.back() == 0xac);

    const auto spk = BuildP2WSHScriptPubKey(witness_script);
    assert(spk.size() == 34 && spk[0] == 0x00 && spk[1] == 0x20);

    const std::string scash_p2wsh = DeriveHtlcP2WSHAddress(witness_script, "scash");
    assert(scash_p2wsh.rfind("scash1q", 0) == 0);
    assert(scash_p2wsh.size() == 65);

    const std::string btc_p2wsh = DeriveHtlcP2WSHAddress(witness_script, "bc");
    assert(btc_p2wsh.rfind("bc1q", 0) == 0);
    assert(btc_p2wsh.size() == 62);

    const auto claim_stack = BuildHtlcClaimWitnessStack(
        {0x30, 0x44, 0x01}, *gen_pub, preimage, witness_script);
    assert(claim_stack.size() == 5);

    const auto extracted = ExtractPreimageFromWitnessItems(claim_stack, secret_hash);
    assert(extracted.has_value() && *extracted == preimage);

    const uint64_t now_ts = 1794500000ULL;
    assert(ValidateAsymmetricTimelocks(now_ts, now_ts + 86400, now_ts + 43200, 21600));
    assert(!ValidateAsymmetricTimelocks(now_ts, now_ts + 45000, now_ts + 43200, 21600));
    assert(!ValidateAsymmetricTimelocks(now_ts, 850000, 840000, 21600));

    const std::string calldata = EncodeErc20HtlcNewContractCalldata(
        "0x1111111111111111111111111111111111111111",
        secret_hash,
        now_ts + 43200,
        "0xdAC17F958D2ee523a2206206994597C13D831ec7",
        25000000ULL);
    assert(calldata.rfind("0x3e6260b8", 0) == 0);
    assert(calldata.size() == 10 + 64 * 5);

    OrderBookEngine book;

    OrderBookEntry ask1;
    ask1.pair = TradingPair::SCASH_USDT;
    ask1.side = OrderSide::SELL_SCASH;
    ask1.price_quote_units = 25000;
    ask1.amount_scash_sats = 500ULL * 100000000ULL;
    ask1.utxo_txid = "aaaa1111";
    ask1.utxo_vout = 0;
    ask1.created_at = 100;
    ask1.expires_at = 999999;

    OrderBookEntry ask2 = ask1;
    ask2.price_quote_units = 24000;
    ask2.amount_scash_sats = 300ULL * 100000000ULL;
    ask2.utxo_txid = "bbbb2222";
    ask2.created_at = 105;

    assert(book.UpsertOrder(ask1));
    assert(book.UpsertOrder(ask2));

    const auto sorted_asks = book.GetAsks(TradingPair::SCASH_USDT);
    assert(sorted_asks.size() == 2);
    assert(sorted_asks[0].price_quote_units == 24000);
    assert(sorted_asks[1].price_quote_units == 25000);

    const auto fills = book.MatchTakerOrder(
        TradingPair::SCASH_USDT, OrderSide::BUY_SCASH, 25000, 600ULL * 100000000ULL);
    assert(fills.size() == 2);
    assert(fills[0].amount_scash_sats == 300ULL * 100000000ULL);
    assert(fills[1].amount_scash_sats == 300ULL * 100000000ULL);

    book.MarkUtxoSpent("bbbb2222", 0);
    assert(book.GetAsks(TradingPair::SCASH_USDT).size() == 1);

    book.RecordOnChainTrade(TradingPair::SCASH_USDT, 0.0240, 300.0, 1800);
    book.RecordOnChainTrade(TradingPair::SCASH_USDT, 0.0265, 150.0, 1820);
    book.RecordOnChainTrade(TradingPair::SCASH_USDT, 0.0235, 100.0, 1840);
    book.RecordOnChainTrade(TradingPair::SCASH_USDT, 0.0252, 250.0, 1855);

    const auto candles = book.GetCandles(TradingPair::SCASH_USDT, 60, 10);
    assert(candles.size() == 1);
    assert(std::abs(candles[0].open - 0.0240) < 1e-9);
    assert(std::abs(candles[0].high - 0.0265) < 1e-9);
    assert(std::abs(candles[0].low - 0.0235) < 1e-9);
    assert(std::abs(candles[0].close - 0.0252) < 1e-9);
    assert(std::abs(candles[0].volume_scash - 800.0) < 1e-9);


    // v8 Tests: Multi-Lot Partial Fills, Per-Lot Secret Isolation, Anti-Option Bond, and Watchtower Splicing
    const auto lots = SplitAmountIntoLots(1000ULL * 100000000ULL, 10, 10000000ULL);
    assert(lots.size() == 10);
    uint64_t lot_sum = 0;
    for (uint64_t v : lots) {
        assert(v == 100ULL * 100000000ULL);
        lot_sum += v;
    }
    assert(lot_sum == 1000ULL * 100000000ULL);

    const auto odd_lots = SplitAmountIntoLots(105500000ULL, 10, 20000000ULL);
    assert(odd_lots.size() == 5);
    assert(odd_lots[0] + odd_lots[1] + odd_lots[2] + odd_lots[3] + odd_lots[4] == 105500000ULL);

    const auto lot0_sec = DeriveIndexedLotPreimage(preimage, 0);
    const auto lot1_sec = DeriveIndexedLotPreimage(preimage, 1);
    assert(lot0_sec != lot1_sec);

    assert(ComputeAntiOptionBondUnits(100000000ULL, 200) == 2000000ULL);

    const std::string dummy_tpl =
        "02000000000101deadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeef00000000";
    const std::string spliced = InjectPreimageIntoClaimTemplate(dummy_tpl, lot0_sec);
    assert(spliced.find(HexEncode(lot0_sec.data(), lot0_sec.size())) != std::string::npos);

    OrderBookEngine lot_book;
    for (uint32_t i = 0; i < 5; ++i) {
        OrderBookEntry le;
        le.parent_order_id = "parent_1000";
        le.lot_index = i;
        le.total_lots = 5;
        le.pair = TradingPair::SCASH_USDT;
        le.side = OrderSide::SELL_SCASH;
        le.price_quote_units = 25000;
        le.amount_scash_sats = 200ULL * 100000000ULL;
        le.utxo_txid = "lot_tx_abc";
        le.utxo_vout = i;
        le.created_at = 200;
        le.expires_at = 999999;
        assert(lot_book.UpsertOrder(le));
    }
    const auto agg_asks = lot_book.GetAggregatedAsks(TradingPair::SCASH_USDT);
    assert(agg_asks.size() == 1);
    assert(agg_asks[0].lot_count == 5);
    assert(agg_asks[0].total_scash_sats == 1000ULL * 100000000ULL);

    // Partial fill of 400 SCASH consumes 2 lots, leaving 600 SCASH (3 lots) on book!
    lot_book.MarkUtxoSpent("lot_tx_abc", 0);
    lot_book.MarkUtxoSpent("lot_tx_abc", 1);
    const auto agg_after = lot_book.GetAggregatedAsks(TradingPair::SCASH_USDT);
    assert(agg_after.size() == 1);
    assert(agg_after[0].lot_count == 3);
    assert(agg_after[0].total_scash_sats == 600ULL * 100000000ULL);


    // v11 Tests: Strict 33-byte compressed secp256k1 pubkey validation (prevents unspendable placeholder hashes)
    assert(IsValidCompressedPubKeyHex("0279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798"));
    assert(!IsValidCompressedPubKeyHex("0x9565eb9935Be659E1bb44EC0622d0962859f6Af6"));
    assert(!IsValidCompressedPubKeyHex("deadbeef"));

    std::cout << "[PASS] All Scash Core 2.4.0 HTLC, scash1q/bc1q P2WSH, USDT ABI, and OrderBook tests succeeded ("
              << scash_p2wsh << ")\n";
    return 0;
}
