Scash version 2.1.0 is now available from:

  https://github.com/ScashNetworkCommunity/scash/releases/tag/scash_v2.1.0-narnia-core-28.4.0

This release upgrades Scash Core from Bitcoin Core 27.0 (scash_v2.0.0-narnia-core-27.0.0) to Bitcoin Core 28.4 (scash_v2.1.0-narnia-core-28.4.0) while preserving all Scash mainnet consensus rules, the 112-byte RandomX v1.2.1 block header, and the aserti3-2d per-block difficulty adjustment algorithm (SIP-0010).

Notable Changes in Scash 2.1.0
==============================
- Rebase onto Bitcoin Core 28.4 (isolating Testnet4/BIP94 rules from Scash networks).
- Blockfile XOR obfuscation enabled by default (-blocksxor=1 / DEFAULT_BLOCKS_XOR = true).
- TRUC (v3) transactions, 1P1C package relay, and Pay-to-Anchor (P2A) enabled while keeping opt-in RBF (-mempoolfullrbf=0 default).
- Legacy single-string warnings RPC output preserved by default alongside JSON-RPC 2.0 updates.
- Fresh Windows installs default to %LOCALAPPDATA%\Scash (AppData\Local\Scash) while preserving existing AppData\Roaming\Scash folders.
- Security backports included for CVE-2024-35202, CVE-2024-52919 (part 2), CVE-2024-52911, CVE-2025-54604, CVE-2025-54605, CVE-2025-46598, and CVE-2025-46597.
- Build System & Toolchain Updates: Linux x86_64 binaries continue to be built on Ubuntu 22.04 LTS (glibc 2.35) for broad distro compatibility while Windows 64-bit cross-compilation and CI verification are upgraded to Ubuntu 24.04 LTS. Includes Clang 18 and GCC 13/14 C++20 static linking fixes (inline base_blob::SetHex in src/uint256.h and -include cstdint in depends/packages/randomx.mk).
