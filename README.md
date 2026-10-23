# Scash Core v2.2.0 "Fillory" (Bitcoin Core v29.3)

**Scash Core v2.2.0 ("Fillory")** upgrades the Scash network daemon, CLI, and Qt GUI wallet to **Bitcoin Core v29.3** while preserving all Scash RandomX Proof-of-Work, ASERT difficulty adjustment (`aserti3-2d`), first-seen mempool (No-RBF), and anti-spam consensus rules.

## What's New in Scash v2.2.0 "Fillory"
- **Upgraded to Bitcoin Core v29.3**: Modern CMake build system (`-B build`), C++20 codebase, ephemeral dust / package relay support, built-in PCP/NAT-PMP port mapping (`-natpmp`), and all upstream stability and security fixes through v29.3.
- **Full Scash Consensus & Feature Integrity**:
  - **RandomX Proof-of-Work & ASERT (`aserti3-2d`)**: Full commitment verification during header sync and disk block loading (`LoadBlockIndexGuts` / `ReadBlockFromDisk`), two-stage `AcceptBlockHeader` validation preventing RandomX VM epoch DoS, and per-block ASERT target updates in `UpdateTime`.
  - **First-Seen Mempool Policy (No-RBF)**: Mempool conflicts are rejected on Scash (`txn-mempool-conflict`), `DEFAULT_WALLET_RBF` is `false`, and `-mempoolfullrbf=1` / `-walletrbf=1` are rejected on Scash chains while allowing legacy `mempoolfullrbf=0` configs.
  - **Anti-Spam & Reorg Protection**: Ordinals dead-code inscription filter (`SCRIPT_VERIFY_DISCOURAGE_ORDINALS`), `DEFAULT_ACCEPT_DATACARRIER = false`, and `-suspiciousreorgdepth` automatic shutdown protection.
  - **Out-of-the-Box Sync**: Built-in DNS seed (`seed.scash.one`), fallback fixed seed nodes, and Mainnet Checkpoint at block **149,000** (`0xd649808dad9fdf227280de34d27d46080c1b8cf55a9f7c3029f3f0a1`).
  - **Antivirus-Friendly Block Storage**: `-blocksxor=1` enabled by default (`DEFAULT_BLOCKS_XOR{true}`).
  - **RPC Backward Compatibility**: `warnings` field in `getblockchaininfo`, `getmininginfo`, and `getnetworkinfo` defaults to a string unless `-nodeprecatedrpc=warnings` is specified; `getmininginfo` reports `"pow_algo": "randomx"` and `getblocktemplate` reports `"rx_epoch_duration"`.
