# Scash version 2.3.0 ("Mid-World" / Bitcoin Core 30.4.0) Release Notes

Scash version `2.3.0` (`v2.3.0-mid-world-core-30.4.0`) is a non-hard-fork upgrade that brings Scash Core from Bitcoin Core `29.3` up to **Bitcoin Core `30.4`**, while keeping 100% consensus compatibility with the existing Scash network, CPU-only RandomX v1.2.1 proof-of-work (`"RandomX-Scash\x01"`), ASERT difficulty adjustment, First-Seen (No-RBF) mempool policy, and anti-spam rules.

Because there are no consensus hard-fork changes, existing wallets, miners (`cpuminer-scash`), mining pools (`miningcore`), and exchanges remain fully compatible.

## Summary of Major Changes in Scash 2.3.0

### 1. Security Fixes Included from Bitcoin Core 30.0 – 30.4
- **CVE-2026-117346 (`#36669`)**: Stores flatfile (`blk*.dat` / `rev*.dat`) positions as 64-bit integers to prevent undo-file overflow corruption during reorgs.
- **CVE-2026-117349 (`#36667`)**: Enforces a timeout interval on paused `send` peers to prevent unbounded memory growth from pre-BIP31 peers.
- **CVE-2026-117350 (`#36665`)**: Aligns socket exception monitoring between `poll()` and `select()` to prevent out-of-band TCP urgent-data memory exhaustion on Windows and macOS.
- **Precomputed Tx Data Lifetime (`#35209`) & LevelDB Compaction (`#35465`)**: Fixes a block validation use-after-free lifetime bug and prevents redundant LevelDB seek compactions on `chainstate`.
- **CVE-2025-54604 & CVE-2025-54605 (`#32604`)**: Rate-limits unconditional disk logging (`1 MiB/hour` per source location) to stop disk-filling attacks from spoofed self-connections or invalid blocks.
- **CVE-2025-46598 (`#31829`) & CVE-2025-46597 (`#32530`)**: Mitigates CPU exhaustion from invalid unconfirmed transactions, overhauls the transaction orphanage with peer/weight limits (`404,000 Wu` per peer), and fixes 32-bit block handling.

### 2. Lower Minimum Transaction & Relay Fees (`0.1 sat/vB`)
- Adopts Bitcoin Core 30's 10x lower minimum relay feerate (`-minrelaytxfee=0.00000100` SCASH/kvB = `0.1 sat/vB`), incremental relay feerate (`0.1 sat/vB`), and minimum block feerate (`0.001 sat/vB`, `#33106`).
- Lowers the default wallet minimum transaction creation fee floor (`DEFAULT_TRANSACTION_MINFEE`) to `0.1 sat/vB` (`100 sat/kvB`) so `scash-qt` and `scash-cli` wallets can create and broadcast low-fee transactions out of the box.

### 3. Scash Anti-Spam, First-Seen Mempool & Legacy RPC Compatibility
- **Blocks `OP_RETURN` Uncapping**: Keeps `DEFAULT_ACCEPT_DATACARRIER = false` (`-datacarrier=0`), keeps `MAX_OP_RETURN_RELAY = 83` (blocking upstream Core 30's `100,000`-byte default), and preserves `SCRIPT_VERIFY_DISCOURAGE_ORDINALS`.
- **Preserves First-Seen (No-RBF) Mempool**: Rejects conflicting mempool transactions (`txn-mempool-conflict`), keeps `DEFAULT_WALLET_RBF = false`, and blocks RBF fee-bumping on Scash networks.
- **Preserves Legacy Wallet & Mining RPC Fields**: Restores `"balance"`, `"unconfirmed_balance"`, and `"immature_balance"` in `getwalletinfo`, keeps `settxfee` enabled by default, and preserves the single-string `"warnings"` field in `getmininginfo` and `getblockchaininfo`.
- **Qt 6 GUI Upgrade (`scash-qt`)**: Upgrades the graphical wallet to Qt 6 with native Windows dark mode while preserving Scash `DummyAddress` support for `SCASHMAIN`, `SCASHTESTNET`, and `SCASHREGTEST`.
