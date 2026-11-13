# Scash version 2.4.0 ("Mid-World" / Bitcoin Core 30.4.0) Release Notes

Scash version `2.4.0` (`scash_v2.4.0-mid-world-core-30.4.0`) upgrades Scash Core `2.3.0` (built on Bitcoin Core `30.4.0`) with a built-in, 100% non-custodial on-chain atomic swap **Exchange** tab (`&Exchange` / `Alt+5`) inside both `scash-qt` and `scash-qtkawaii`. This release maintains full consensus compatibility with the existing Scash mainnet (`RandomX v1.2.1` CPU-only Proof-of-Work, ASERT difficulty adjustment, First-Seen No-RBF mempool policy, and `0.1 sat/vB` = `0.00000100 SCASH/kvB` minimum relay fee).

## Summary of Major Changes in Scash 2.4.0

### 1. Native On-Chain HTLC Exchange Tab (`SCASH/USDT` & `SCASH/BTC`)
- **Zero Sidechains or Custodians**: Coins lock directly on their native Layer-1 blockchains using canonical BIP-199 SegWit v0 `P2WSH` Hashed Timelock Contracts (`OP_SHA256` + `OP_CHECKLOCKTIMEVERIFY`) on Scash and Bitcoin, paired with the audited non-custodial `ScashHTLCERC20.sol` SHA-256 timelock contract (`contrib/dex/ScashHTLCERC20.sol`) for native Tether `USDT`.
- **Automatic On-Chain Resolution & Asymmetric CLTV Safety**: Enforces asymmetric maker/taker `OP_CHECKLOCKTIMEVERIFY` windows (`>= 12h` maker lock vs. half-duration taker counter-lock) with automatic preimage extraction from witness stacks and post-expiry refund recovery.
- **Full Exchange Workspace**: Real-time `QPainter` OHLCV candlestick chart (`1m`, `5m`, `15m`, `1h`, `4h`, `1d`) with `EMA-9` / `EMA-21` overlays and volume histogram, live depth-shaded Bid/Ask order book, order lock ticket, and active HTLC settlement monitor.

### 2. Dual `scash-qt` & `scash-qtkawaii` Theme Support & P2P Order Mesh
- **Seamless `scash-qtkawaii` Styling**: Automatically detects `scash-qtkawaii` / `scash-qtkawaii.exe` execution (as well as runtime Kawaii stylesheet switches) to apply dark-violet, mint, and teal Kawaii styling without configuration changes.
- **Localhost Node & Decentralized Gossip Mesh**: Connects in-process via `ClientModel` / `WalletModel` and localhost JSON-RPC (`127.0.0.1:8332` `.cookie` auto-auth) while gossiping signed orders over TCP port `8345` with `gettxout` UTXO proof-of-funds verification.

### 3. Build System & Runtime Reliability Improvements
- Restored header-only `src/boost/compute/detail/lru_cache.hpp` and `Boost.Optional` in `depends/packages/boost.mk` for deterministic Linux (`x86_64-pc-linux-gnu`) and Windows 11 (`x86_64-w64-mingw32`) static `depends` builds under Bitcoin Core `30.4.0`.
- Replaced `QNetworkAccessManager` RPC transport with native `QTcpSocket` HTTP/1.1 JSON-RPC so static `depends` Qt 6 builds operate with zero external OpenSSL/HTTP feature dependencies.
