# Scash v2.2.0 "Fillory" Release Notes (Bitcoin Core v29.3 Upgrade)

Scash Core version **2.2.0 ("Fillory")** is a major upgrade rebasing Scash onto **Bitcoin Core v29.3**.

## Summary of Key Changes
1. **Bitcoin Core v29.3 Base & CMake Build System**: Migrates from Autotools to CMake (`cmake -B build`) and C++20.
2. **RandomX & ASERT Consensus Preservation**: Preserves all Scash RandomX PoW and per-block ASERT difficulty adjustment rules while isolating Scash from Bitcoin's 2,016-block BIP94 timewarp retarget assumptions in `getmininginfo` and `getblocktemplate`.
3. **First-Seen Mempool (No-RBF) & Anti-Spam**: Retains Scash's first-seen rule rejecting conflicting mempool transactions (`txn-mempool-conflict`), Ordinals pattern discouragement (`SCRIPT_VERIFY_DISCOURAGE_ORDINALS`), and `-suspiciousreorgdepth` protection.
4. **Mainnet Checkpoint 149,000 & Built-In Seeds**: Adds checkpoint `149,000` (`0xd649808dad9fdf227280de34d27d45b69f3d46080c1b8cf55a9f7c3029f3f0a1`), `seed.scash.one`, and fallback seed peers.
5. **RPC & Windows Compatibility**: Defaults `-blocksxor=1`, preserves string `warnings` RPC output by default, and uses `%LOCALAPPDATA%\Scash` for fresh Windows installs while automatically detecting existing `%APPDATA%\Scash` data directories.
