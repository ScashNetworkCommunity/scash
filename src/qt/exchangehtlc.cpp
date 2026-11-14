// Copyright (c) 2026 The Scash Network Community developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "exchangehtlc.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <iomanip>
#include <random>
#include <sstream>
#include <utility>

#if !defined(SCASH_DEX_STANDALONE_TEST) && __has_include(<QRandomGenerator>)
#define SCASH_DEX_USE_QT_CSPRNG 1
#include <QRandomGenerator>
#else
#define SCASH_DEX_USE_QT_CSPRNG 0
#endif

namespace scash_dex {

namespace {

// Bitcoin Script opcodes (identical in Scash Core 2.3.0 / 2.4.0 / Bitcoin Core 30)
constexpr uint8_t kOpIf = 0x63;
constexpr uint8_t kOpElse = 0x67;
constexpr uint8_t kOpEndIf = 0x68;
constexpr uint8_t kOpDrop = 0x75;
constexpr uint8_t kOpDup = 0x76;
constexpr uint8_t kOpEqualVerify = 0x88;
constexpr uint8_t kOpSha256 = 0xa8;
constexpr uint8_t kOpHash160 = 0xa9;
constexpr uint8_t kOpCheckSig = 0xac;
constexpr uint8_t kOpCheckLockTimeVerify = 0xb1;

constexpr uint32_t RotR32(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
constexpr uint32_t RotL32(uint32_t x, uint32_t n) { return (x << n) | (x >> (32 - n)); }

std::array<uint8_t, 32> PortableSha256(const uint8_t* data, size_t len)
{
    static constexpr uint32_t k[64] = {
        0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL, 0x3956c25bUL, 0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL,
        0xd807aa98UL, 0x12835b01UL, 0x243185beUL, 0x550c7dc3UL, 0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL, 0xc19bf174UL,
        0xe49b69c1UL, 0xefbe4786UL, 0x0fc19dc6UL, 0x240ca1ccUL, 0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL,
        0x983e5152UL, 0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL, 0xc6e00bf3UL, 0xd5a79147UL, 0x06ca6351UL, 0x14292967UL,
        0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL, 0x53380d13UL, 0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
        0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL, 0xd192e819UL, 0xd6990624UL, 0xf40e3585UL, 0x106aa070UL,
        0x19a4c116UL, 0x1e376c08UL, 0x2748774cUL, 0x34b0bcb5UL, 0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL, 0x682e6ff3UL,
        0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL, 0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL};

    uint32_t h[8] = {
        0x6a09e667UL, 0xbb67ae85UL, 0x3c6ef372UL, 0xa54ff53aUL,
        0x510e527fUL, 0x9b05688cUL, 0x1f83d9abUL, 0x5be0cd19UL};

    std::vector<uint8_t> msg(data, data + len);
    msg.push_back(0x80);
    while ((msg.size() % 64) != 56) msg.push_back(0x00);
    const uint64_t bit_len = static_cast<uint64_t>(len) * 8ULL;
    for (int i = 7; i >= 0; --i) msg.push_back(static_cast<uint8_t>((bit_len >> (i * 8)) & 0xff));

    for (size_t chunk = 0; chunk < msg.size(); chunk += 64) {
        uint32_t w[64]{};
        for (int i = 0; i < 16; ++i) {
            w[i] = (static_cast<uint32_t>(msg[chunk + i * 4]) << 24) |
                   (static_cast<uint32_t>(msg[chunk + i * 4 + 1]) << 16) |
                   (static_cast<uint32_t>(msg[chunk + i * 4 + 2]) << 8) |
                   static_cast<uint32_t>(msg[chunk + i * 4 + 3]);
        }
        for (int i = 16; i < 64; ++i) {
            const uint32_t s0 = RotR32(w[i - 15], 7) ^ RotR32(w[i - 15], 18) ^ (w[i - 15] >> 3);
            const uint32_t s1 = RotR32(w[i - 2], 17) ^ RotR32(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }
        uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];
        for (int i = 0; i < 64; ++i) {
            const uint32_t S1 = RotR32(e, 6) ^ RotR32(e, 11) ^ RotR32(e, 25);
            const uint32_t ch = (e & f) ^ ((~e) & g);
            const uint32_t temp1 = hh + S1 + ch + k[i] + w[i];
            const uint32_t S0 = RotR32(a, 2) ^ RotR32(a, 13) ^ RotR32(a, 22);
            const uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            const uint32_t temp2 = S0 + maj;
            hh = g; g = f; f = e; e = d + temp1;
            d = c; c = b; b = a; a = temp1 + temp2;
        }
        h[0] += a; h[1] += b; h[2] += c; h[3] += d;
        h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
    }

    std::array<uint8_t, 32> out{};
    for (int i = 0; i < 8; ++i) {
        out[i * 4] = static_cast<uint8_t>((h[i] >> 24) & 0xff);
        out[i * 4 + 1] = static_cast<uint8_t>((h[i] >> 16) & 0xff);
        out[i * 4 + 2] = static_cast<uint8_t>((h[i] >> 8) & 0xff);
        out[i * 4 + 3] = static_cast<uint8_t>(h[i] & 0xff);
    }
    return out;
}

std::array<uint8_t, 20> PortableRipemd160(const uint8_t* data, size_t len)
{
    static constexpr int r1[80] = {
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15, 7,4,13,1,10,6,15,3,12,0,9,5,2,14,11,8,
        3,10,14,4,9,15,8,1,2,7,0,6,13,11,5,12, 1,9,11,10,0,8,12,4,13,3,7,15,14,5,6,2,
        4,0,5,9,7,12,2,10,14,1,3,8,11,6,15,13};
    static constexpr int r2[80] = {
        5,14,7,0,9,2,11,4,13,6,15,8,1,10,3,12, 6,11,3,7,0,13,5,10,14,15,8,12,4,9,1,2,
        15,5,1,3,7,14,6,9,11,8,12,2,10,0,4,13, 8,6,4,1,3,11,15,0,5,12,2,13,9,7,10,14,
        12,15,10,4,1,5,8,7,6,2,13,14,0,3,9,11};
    static constexpr int s1[80] = {
        11,14,15,12,5,8,7,9,11,13,14,15,6,7,9,8, 7,6,8,13,11,9,7,15,7,12,15,9,11,7,13,12,
        11,13,6,7,14,9,13,15,14,8,13,6,5,12,7,5, 11,12,14,15,14,15,9,8,9,14,5,6,8,6,5,12,
        9,15,5,11,6,8,13,12,5,12,13,14,11,8,5,6};
    static constexpr int s2[80] = {
        8,9,9,11,13,15,15,5,7,7,8,11,14,14,12,6, 9,13,15,7,12,8,9,11,7,7,12,7,6,15,13,11,
        9,7,15,11,8,6,6,14,12,13,5,14,13,13,7,5, 15,5,8,11,14,14,6,14,6,9,12,9,12,5,15,8,
        8,5,12,9,12,5,14,6,8,13,6,5,15,13,11,11};

    auto f = [](int j, uint32_t x, uint32_t y, uint32_t z) -> uint32_t {
        if (j < 16) return x ^ y ^ z;
        if (j < 32) return (x & y) | ((~x) & z);
        if (j < 48) return (x | (~y)) ^ z;
        if (j < 64) return (x & z) | (y & (~z));
        return x ^ (y | (~z));
    };
    static constexpr uint32_t k1[5] = {0x00000000UL, 0x5a827999UL, 0x6ed9eba1UL, 0x8f1bbcdcUL, 0xa953fd4eUL};
    static constexpr uint32_t k2[5] = {0x50a28be6UL, 0x5c4dd124UL, 0x6d703ef3UL, 0x7a6d76e9UL, 0x00000000UL};

    uint32_t h0 = 0x67452301UL, h1 = 0xefcdab89UL, h2 = 0x98badcfeUL, h3 = 0x10325476UL, h4 = 0xc3d2e1f0UL;

    std::vector<uint8_t> msg(data, data + len);
    msg.push_back(0x80);
    while ((msg.size() % 64) != 56) msg.push_back(0x00);
    const uint64_t bit_len = static_cast<uint64_t>(len) * 8ULL;
    for (int i = 0; i < 8; ++i) msg.push_back(static_cast<uint8_t>((bit_len >> (i * 8)) & 0xff));

    for (size_t chunk = 0; chunk < msg.size(); chunk += 64) {
        uint32_t x[16]{};
        for (int i = 0; i < 16; ++i) {
            x[i] = static_cast<uint32_t>(msg[chunk + i * 4]) |
                   (static_cast<uint32_t>(msg[chunk + i * 4 + 1]) << 8) |
                   (static_cast<uint32_t>(msg[chunk + i * 4 + 2]) << 16) |
                   (static_cast<uint32_t>(msg[chunk + i * 4 + 3]) << 24);
        }
        uint32_t al = h0, bl = h1, cl = h2, dl = h3, el = h4;
        uint32_t ar = h0, br = h1, cr = h2, dr = h3, er = h4;
        for (int j = 0; j < 80; ++j) {
            uint32_t tl = RotL32(al + f(j, bl, cl, dl) + x[r1[j]] + k1[j / 16], s1[j]) + el;
            al = el; el = dl; dl = RotL32(cl, 10); cl = bl; bl = tl;

            uint32_t tr = RotL32(ar + f(79 - j, br, cr, dr) + x[r2[j]] + k2[j / 16], s2[j]) + er;
            ar = er; er = dr; dr = RotL32(cr, 10); cr = br; br = tr;
        }
        uint32_t t = h1 + cl + dr;
        h1 = h2 + dl + er;
        h2 = h3 + el + ar;
        h3 = h4 + al + br;
        h4 = h0 + bl + cr;
        h0 = t;
    }

    std::array<uint8_t, 20> out{};
    const uint32_t words[5] = {h0, h1, h2, h3, h4};
    for (int i = 0; i < 5; ++i) {
        out[i * 4] = static_cast<uint8_t>(words[i] & 0xff);
        out[i * 4 + 1] = static_cast<uint8_t>((words[i] >> 8) & 0xff);
        out[i * 4 + 2] = static_cast<uint8_t>((words[i] >> 16) & 0xff);
        out[i * 4 + 3] = static_cast<uint8_t>((words[i] >> 24) & 0xff);
    }
    return out;
}

std::vector<uint8_t> EncodeScriptNum(int64_t value)
{
    if (value == 0) return {};
    std::vector<uint8_t> result;
    const bool neg = (value < 0);
    uint64_t absvalue = neg ? static_cast<uint64_t>(-value) : static_cast<uint64_t>(value);
    while (absvalue > 0) {
        result.push_back(static_cast<uint8_t>(absvalue & 0xff));
        absvalue >>= 8;
    }
    if (result.back() & 0x80) {
        result.push_back(neg ? 0x80 : 0x00);
    } else if (neg) {
        result.back() |= 0x80;
    }
    return result;
}

void AppendPushData(std::vector<uint8_t>& script, const uint8_t* data, size_t len)
{
    if (len == 0) {
        script.push_back(0x00);
        return;
    }
    if (len == 1 && data[0] >= 1 && data[0] <= 16) {
        script.push_back(static_cast<uint8_t>(0x50 + data[0]));
        return;
    }
    if (len < 0x4c) {
        script.push_back(static_cast<uint8_t>(len));
    } else if (len <= 0xff) {
        script.push_back(0x4c);
        script.push_back(static_cast<uint8_t>(len));
    } else {
        script.push_back(0x4d);
        script.push_back(static_cast<uint8_t>(len & 0xff));
        script.push_back(static_cast<uint8_t>((len >> 8) & 0xff));
    }
    script.insert(script.end(), data, data + len);
}

uint32_t Bech32Polymod(const std::vector<uint8_t>& values)
{
    uint32_t chk = 1;
    for (uint8_t v : values) {
        uint8_t top = static_cast<uint8_t>(chk >> 25);
        chk = ((chk & 0x1ffffffUL) << 5) ^ v;
        if (top & 1) chk ^= 0x3b6a57b2UL;
        if (top & 2) chk ^= 0x26508e6dUL;
        if (top & 4) chk ^= 0x1ea119faUL;
        if (top & 8) chk ^= 0x3d4233ddUL;
        if (top & 16) chk ^= 0x2a1462b3UL;
    }
    return chk;
}

std::vector<uint8_t> ExpandHrp(const std::string& hrp)
{
    std::vector<uint8_t> ret;
    ret.reserve(hrp.size() * 2 + 1);
    for (char c : hrp) ret.push_back(static_cast<uint8_t>(c) >> 5);
    ret.push_back(0);
    for (char c : hrp) ret.push_back(static_cast<uint8_t>(c) & 0x1f);
    return ret;
}

std::string StripHexPrefix(const std::string& s)
{
    if (s.size() >= 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        return s.substr(2);
    }
    return s;
}

std::string PadAbiWordLeft(const std::string& clean_hex)
{
    if (clean_hex.size() >= 64) return clean_hex.substr(clean_hex.size() - 64);
    return std::string(64 - clean_hex.size(), '0') + clean_hex;
}

std::string EncodeUint64AbiWord(uint64_t val)
{
    std::ostringstream oss;
    oss << std::hex << std::nouppercase << val;
    return PadAbiWordLeft(oss.str());
}

} // namespace

std::array<uint8_t, 32> Sha256Bytes(const uint8_t* data, size_t len)
{
    return PortableSha256(data, len);
}

std::array<uint8_t, 32> Sha256Bytes(const std::vector<uint8_t>& data)
{
    return Sha256Bytes(data.data(), data.size());
}

std::array<uint8_t, 20> Hash160Bytes(const std::vector<uint8_t>& data)
{
    const auto sha = Sha256Bytes(data);
    return PortableRipemd160(sha.data(), sha.size());
}

std::array<uint8_t, 32> GenerateRandomPreimage()
{
    std::array<uint8_t, 32> secret{};
#if SCASH_DEX_USE_QT_CSPRNG
    uint32_t words[8]{};
    QRandomGenerator::system()->fillRange(words, 8);
    std::memcpy(secret.data(), words, 32);
#else
    std::random_device rd;
    for (size_t i = 0; i < secret.size(); i += 4) {
        const uint32_t v = rd();
        secret[i] = static_cast<uint8_t>(v & 0xff);
        secret[i + 1] = static_cast<uint8_t>((v >> 8) & 0xff);
        secret[i + 2] = static_cast<uint8_t>((v >> 16) & 0xff);
        secret[i + 3] = static_cast<uint8_t>((v >> 24) & 0xff);
    }
#endif
    return secret;
}

std::string HexEncode(const uint8_t* data, size_t len)
{
    static constexpr char kHex[] = "0123456789abcdef";
    std::string out;
    out.reserve(len * 2);
    for (size_t i = 0; i < len; ++i) {
        out.push_back(kHex[(data[i] >> 4) & 0x0f]);
        out.push_back(kHex[data[i] & 0x0f]);
    }
    return out;
}

std::string HexEncode(const std::vector<uint8_t>& data)
{
    return HexEncode(data.data(), data.size());
}

std::optional<std::vector<uint8_t>> HexDecode(const std::string& raw)
{
    const std::string hex = StripHexPrefix(raw);
    if (hex.size() % 2 != 0) return std::nullopt;
    auto nibble = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };
    std::vector<uint8_t> out;
    out.reserve(hex.size() / 2);
    for (size_t i = 0; i < hex.size(); i += 2) {
        int hi = nibble(hex[i]);
        int lo = nibble(hex[i + 1]);
        if (hi < 0 || lo < 0) return std::nullopt;
        out.push_back(static_cast<uint8_t>((hi << 4) | lo));
    }
    return out;
}

std::string EncodeBech32P2WSH(const std::string& hrp, const std::array<uint8_t, 32>& witness_script_sha256)
{
    static constexpr char kCharset[] = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";
    std::vector<uint8_t> data5;
    data5.reserve(1 + 52);
    data5.push_back(0);

    uint32_t acc = 0;
    int bits = 0;
    for (uint8_t byte : witness_script_sha256) {
        acc = (acc << 8) | byte;
        bits += 8;
        while (bits >= 5) {
            bits -= 5;
            data5.push_back(static_cast<uint8_t>((acc >> bits) & 0x1f));
        }
    }
    if (bits > 0) {
        data5.push_back(static_cast<uint8_t>((acc << (5 - bits)) & 0x1f));
    }

    std::vector<uint8_t> chk_input = ExpandHrp(hrp);
    chk_input.insert(chk_input.end(), data5.begin(), data5.end());
    chk_input.insert(chk_input.end(), 6, 0);
    const uint32_t mod = Bech32Polymod(chk_input) ^ 1;

    std::string ret = hrp + "1";
    for (uint8_t v : data5) ret.push_back(kCharset[v]);
    for (int i = 0; i < 6; ++i) {
        ret.push_back(kCharset[(mod >> (5 * (5 - i))) & 0x1f]);
    }
    return ret;
}

std::vector<uint8_t> BuildHtlcWitnessScript(const HtlcParameters& params)
{
    std::vector<uint8_t> script;
    script.reserve(112);

    script.push_back(kOpIf);
    script.push_back(kOpSha256);
    AppendPushData(script, params.secret_hash.data(), params.secret_hash.size());
    script.push_back(kOpEqualVerify);
    script.push_back(kOpDup);
    script.push_back(kOpHash160);
    AppendPushData(script, params.recipient_pubkey_hash.data(), params.recipient_pubkey_hash.size());

    script.push_back(kOpElse);
    const std::vector<uint8_t> lock_num = EncodeScriptNum(static_cast<int64_t>(params.cltv_expiry));
    AppendPushData(script, lock_num.data(), lock_num.size());
    script.push_back(kOpCheckLockTimeVerify);
    script.push_back(kOpDrop);
    script.push_back(kOpDup);
    script.push_back(kOpHash160);
    AppendPushData(script, params.refund_pubkey_hash.data(), params.refund_pubkey_hash.size());

    script.push_back(kOpEndIf);
    script.push_back(kOpEqualVerify);
    script.push_back(kOpCheckSig);

    return script;
}

std::vector<uint8_t> BuildP2WSHScriptPubKey(const std::vector<uint8_t>& witness_script)
{
    const auto scripthash = Sha256Bytes(witness_script);
    std::vector<uint8_t> spk;
    spk.reserve(34);
    spk.push_back(0x00);
    spk.push_back(0x20);
    spk.insert(spk.end(), scripthash.begin(), scripthash.end());
    return spk;
}

std::string DeriveHtlcP2WSHAddress(const std::vector<uint8_t>& witness_script, const std::string& hrp)
{
    const auto scripthash = Sha256Bytes(witness_script);
    return EncodeBech32P2WSH(hrp, scripthash);
}

bool ValidateAsymmetricTimelocks(uint64_t now_sec,
                                 uint32_t maker_cltv_sec,
                                 uint32_t taker_cltv_sec,
                                 uint32_t min_safety_delta_sec)
{
    if (maker_cltv_sec < 500000000U || taker_cltv_sec < 500000000U) {
        return false;
    }
    if (taker_cltv_sec <= now_sec + (min_safety_delta_sec / 2)) {
        return false;
    }
    if (maker_cltv_sec < taker_cltv_sec + min_safety_delta_sec) {
        return false;
    }
    return true;
}

std::vector<std::vector<uint8_t>> BuildHtlcClaimWitnessStack(
    const std::vector<uint8_t>& der_sig_with_sighash,
    const std::vector<uint8_t>& recipient_pubkey,
    const std::array<uint8_t, 32>& secret_preimage,
    const std::vector<uint8_t>& witness_script)
{
    std::vector<uint8_t> preimage_vec(secret_preimage.begin(), secret_preimage.end());
    std::vector<uint8_t> true_branch{0x01};
    return {
        der_sig_with_sighash,
        recipient_pubkey,
        preimage_vec,
        true_branch,
        witness_script,
    };
}

std::vector<std::vector<uint8_t>> BuildHtlcRefundWitnessStack(
    const std::vector<uint8_t>& der_sig_with_sighash,
    const std::vector<uint8_t>& refund_pubkey,
    const std::vector<uint8_t>& witness_script)
{
    std::vector<uint8_t> false_branch{};
    return {
        der_sig_with_sighash,
        refund_pubkey,
        false_branch,
        witness_script,
    };
}

std::optional<std::array<uint8_t, 32>> ExtractPreimageFromWitnessItems(
    const std::vector<std::vector<uint8_t>>& witness_items,
    const std::array<uint8_t, 32>& expected_secret_hash)
{
    for (const auto& item : witness_items) {
        if (item.size() == 32) {
            const auto candidate_hash = Sha256Bytes(item);
            if (candidate_hash == expected_secret_hash) {
                std::array<uint8_t, 32> found{};
                std::memcpy(found.data(), item.data(), 32);
                return found;
            }
        }
    }
    return std::nullopt;
}

std::string EncodeErc20HtlcNewContractCalldata(
    const std::string& receiver_evm_address_hex,
    const std::array<uint8_t, 32>& secret_hash,
    uint64_t timelock_unix_sec,
    const std::string& usdt_token_contract_hex,
    uint64_t amount_usdt_micro)
{
    std::string recv_clean = StripHexPrefix(receiver_evm_address_hex);
    std::string token_clean = StripHexPrefix(usdt_token_contract_hex);
    std::transform(recv_clean.begin(), recv_clean.end(), recv_clean.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    std::transform(token_clean.begin(), token_clean.end(), token_clean.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    return "0x3e6260b8" +
           PadAbiWordLeft(recv_clean) +
           HexEncode(secret_hash.data(), secret_hash.size()) +
           EncodeUint64AbiWord(timelock_unix_sec) +
           PadAbiWordLeft(token_clean) +
           EncodeUint64AbiWord(amount_usdt_micro);
}

std::string EncodeErc20HtlcWithdrawCalldata(
    const std::array<uint8_t, 32>& contract_id,
    const std::array<uint8_t, 32>& secret_preimage)
{
    return "0x9c7b8f6f" +
           HexEncode(contract_id.data(), contract_id.size()) +
           HexEncode(secret_preimage.data(), secret_preimage.size());
}

std::string EncodeErc20HtlcRefundCalldata(
    const std::array<uint8_t, 32>& contract_id)
{
    return "0x7249fbb6" + HexEncode(contract_id.data(), contract_id.size());
}

std::string BuildCanonicalOrderSigningMessage(const OrderBookEntry& entry)
{
    std::ostringstream oss;
    oss << "SCASH-DEX-v2.4"
        << "|pair=" << (entry.pair == TradingPair::SCASH_USDT ? "SCASH/USDT" : "SCASH/BTC")
        << "|side=" << (entry.side == OrderSide::SELL_SCASH ? "SELL" : "BUY")
        << "|price=" << entry.price_quote_units
        << "|amount_sats=" << entry.amount_scash_sats
        << "|utxo=" << entry.utxo_txid << ":" << entry.utxo_vout
        << "|recv=" << entry.maker_counterparty_address
        << "|exp=" << entry.expires_at;
    return oss.str();
}

std::string ComputeOrderId(const OrderBookEntry& entry)
{
    const std::string msg = BuildCanonicalOrderSigningMessage(entry);
    const auto digest = Sha256Bytes(reinterpret_cast<const uint8_t*>(msg.data()), msg.size());
    return HexEncode(digest.data(), digest.size());
}

bool OrderBookEngine::UpsertOrder(OrderBookEntry order)
{
    if (order.price_quote_units == 0 || order.amount_scash_sats == 0) {
        return false;
    }
    if (order.order_id.empty()) {
        order.order_id = ComputeOrderId(order);
    }
    for (auto& existing : m_orders) {
        if (existing.order_id == order.order_id) {
            existing = std::move(order);
            return true;
        }
    }
    m_orders.push_back(std::move(order));
    return true;
}

bool OrderBookEngine::RemoveOrder(const std::string& order_id)
{
    const auto prev_size = m_orders.size();
    m_orders.erase(
        std::remove_if(m_orders.begin(), m_orders.end(),
                       [&](const OrderBookEntry& e) { return e.order_id == order_id; }),
        m_orders.end());
    return m_orders.size() != prev_size;
}

void OrderBookEngine::MarkUtxoSpent(const std::string& txid, uint32_t vout)
{
    if (txid.empty()) return;
    m_orders.erase(
        std::remove_if(m_orders.begin(), m_orders.end(),
                       [&](const OrderBookEntry& e) {
                           return e.utxo_txid == txid && e.utxo_vout == vout;
                       }),
        m_orders.end());
}

std::vector<OrderBookEntry> OrderBookEngine::GetAsks(TradingPair pair, size_t limit) const
{
    std::vector<OrderBookEntry> asks;
    for (const auto& o : m_orders) {
        if (o.pair == pair && o.side == OrderSide::SELL_SCASH) {
            asks.push_back(o);
        }
    }
    std::sort(asks.begin(), asks.end(), [](const OrderBookEntry& a, const OrderBookEntry& b) {
        if (a.price_quote_units != b.price_quote_units) {
            return a.price_quote_units < b.price_quote_units;
        }
        return a.created_at < b.created_at;
    });
    if (asks.size() > limit) asks.resize(limit);
    return asks;
}

std::vector<OrderBookEntry> OrderBookEngine::GetBids(TradingPair pair, size_t limit) const
{
    std::vector<OrderBookEntry> bids;
    for (const auto& o : m_orders) {
        if (o.pair == pair && o.side == OrderSide::BUY_SCASH) {
            bids.push_back(o);
        }
    }
    std::sort(bids.begin(), bids.end(), [](const OrderBookEntry& a, const OrderBookEntry& b) {
        if (a.price_quote_units != b.price_quote_units) {
            return a.price_quote_units > b.price_quote_units;
        }
        return a.created_at < b.created_at;
    });
    if (bids.size() > limit) bids.resize(limit);
    return bids;
}

std::vector<OrderBookEntry> OrderBookEngine::MatchTakerOrder(
    TradingPair pair,
    OrderSide taker_side,
    uint64_t limit_price_quote_units,
    uint64_t max_scash_sats) const
{
    std::vector<OrderBookEntry> matched;
    uint64_t remaining = max_scash_sats;

    if (taker_side == OrderSide::BUY_SCASH) {
        const auto asks = GetAsks(pair, m_orders.size());
        for (const auto& ask : asks) {
            if (ask.price_quote_units > limit_price_quote_units || remaining == 0) break;
            OrderBookEntry slice = ask;
            if (slice.amount_scash_sats > remaining) {
                slice.amount_scash_sats = remaining;
            }
            remaining -= slice.amount_scash_sats;
            matched.push_back(std::move(slice));
        }
    } else {
        const auto bids = GetBids(pair, m_orders.size());
        for (const auto& bid : bids) {
            if (bid.price_quote_units < limit_price_quote_units || remaining == 0) break;
            OrderBookEntry slice = bid;
            if (slice.amount_scash_sats > remaining) {
                slice.amount_scash_sats = remaining;
            }
            remaining -= slice.amount_scash_sats;
            matched.push_back(std::move(slice));
        }
    }
    return matched;
}

void OrderBookEngine::RecordOnChainTrade(TradingPair pair,
                                         double price,
                                         double volume_scash,
                                         uint64_t timestamp_sec)
{
    if (price <= 0.0 || volume_scash <= 0.0) return;
    m_trades.push_back(TradePrint{pair, price, volume_scash, timestamp_sec});
    std::sort(m_trades.begin(), m_trades.end(), [](const TradePrint& a, const TradePrint& b) {
        return a.timestamp_sec < b.timestamp_sec;
    });
}

std::vector<OhlcvCandle> OrderBookEngine::GetCandles(TradingPair pair,
                                                     uint32_t bucket_seconds,
                                                     size_t max_candles) const
{
    if (bucket_seconds == 0) bucket_seconds = 60;
    std::vector<OhlcvCandle> candles;

    for (const auto& t : m_trades) {
        if (t.pair != pair) continue;
        const uint64_t bucket = (t.timestamp_sec / bucket_seconds) * bucket_seconds;
        if (candles.empty() || candles.back().bucket_start_sec != bucket) {
            candles.push_back(OhlcvCandle{
                bucket,
                t.price,
                t.price,
                t.price,
                t.price,
                t.volume_scash,
            });
        } else {
            auto& c = candles.back();
            c.high = std::max(c.high, t.price);
            c.low = std::min(c.low, t.price);
            c.close = t.price;
            c.volume_scash += t.volume_scash;
        }
    }

    if (candles.size() > max_candles) {
        candles.erase(candles.begin(), candles.end() - static_cast<std::ptrdiff_t>(max_candles));
    }
    return candles;
}

} // namespace scash_dex
