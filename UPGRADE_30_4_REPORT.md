# Scash 2.3.0 (Mid-World) — Bitcoin Core v30.4 Build & Verification Report

- **Branch**: `scash-30.4-Mid-World`
- **Version**: `v2.3.0-mid-world-core-30.4.0` (Non-hard-fork upgrade on Bitcoin Core 30.4)
- **RandomX**: `ScashNetworkCommunity/RandomX@scash` (v1.2.1 with `RANDOMX_ARGON_SALT="RandomX-Scash\x01"`)

## 1. CMake Configure Summary (exit code: 0)
```text
Preprocessor defined macros ........... 
C++ compiler flags .................... -O2 -g -std=c++20 -fPIC -fmacro-prefix-map=/home/runner/work/scash/scash/src=. -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3 -Wstack-protector -fstack-protector-all -fcf-protection=full -fstack-clash-protection -Wall -Wextra -Wgnu -Wformat -Wformat-security -Wvla -Wshadow-field -Wthread-safety -Wloop-analysis -Wredundant-decls -Wunused-member-function -Wdate-time -Wconditional-uninitialized -Woverloaded-virtual -Wsuggest-override -Wimplicit-fallthrough -Wunreachable-code -Wdocumentation -Wself-assign -Wundef -Wno-unused-parameter
Linker flags .......................... -O2 -g -fstack-protector-all -fcf-protection=full -fstack-clash-protection -Wl,-z,relro -Wl,-z,now -Wl,-z,separate-code -fPIE -pie

NOTE: The summary above may not exactly match the final applied build flags
      if any additional CMAKE_* or environment variables have been modified.
      To see the exact flags applied, build with the --verbose option.

Treat compiler warnings as errors ..... OFF
Use ccache for compiling .............. ON


-- Configuring done (25.5s)
-- Generating done (0.3s)
-- Build files have been written to: /home/runner/work/scash/scash/build
```

## 2. Build & Unit Test Outcome (exit code: 0)
**BUILD SUCCEEDED (scashd + scash-qt + scash-cli + test_scash)!**
```text
Scash daemon version v2.3.0-mid-world-core-30.4.0 bitcoind
Copyright (C) 2009-2026 The Scash developers
Copyright (C) 2009-2026 The Bitcoin Core developers
Scash RPC client version v2.3.0-mid-world-core-30.4.0
Copyright (C) 2009-2026 The Scash developers
Copyright (C) 2009-2026 The Bitcoin Core developers
--- Unit Test Suite (79 test cases) ---
Running 79 test cases...

[1;32;49m*** No errors detected
[0;39;49m--- Live Scash Regtest Check (0.1 sat/vB minrelaytxfee, OP_RETURN blocked, & legacy getwalletinfo fields) ---
Scash starting
[getmempoolinfo]
  "loaded": true,
  "minrelaytxfee": 0.00000100,
  "incrementalrelayfee": 0.00000100,
  "maxdatacarriersize": 0
[createwallet & getwalletinfo legacy fields]
  "walletname": "testwallet",
  "balance": 0.00000000,
  "unconfirmed_balance": 0.00000000,
  "immature_balance": 0.00000000,
[getmininginfo]
  "chain": "scashregtest",
  "pow_algo": "randomx",
  "warnings": ""
Scash stopping
```
