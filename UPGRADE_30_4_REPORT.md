# Scash 3.0.0 (Mid-World) — Bitcoin Core v30.4 + RandomX v2 Build & Test Report

- **Branch**: `scash-30.4-Mid-World`
- **Target Core Release**: `v30.4` (Scash 3.0.0 "Mid-World")
- **RandomX Branch**: `ScashNetworkCommunity/RandomX@scash-v2` (`RANDOMX_PROGRAM_SIZE_V2=384`, `RANDOMX_FLAG_V2=128`)

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


-- Configuring done (20.5s)
-- Generating done (0.3s)
-- Build files have been written to: /home/runner/work/scash/scash/build
```

## 2. Build & Unit Test Outcome (exit code: 0)
**BUILD SUCCEEDED (scashd + scash-qt + scash-cli + test_scash)!**
```text
Scash daemon version v3.0.0-mid-world-core-30.4.0 bitcoind
Copyright (C) 2009-2026 The Scash developers
Copyright (C) 2009-2026 The Bitcoin Core developers
Scash RPC client version v3.0.0-mid-world-core-30.4.0
Copyright (C) 2009-2026 The Scash developers
Copyright (C) 2009-2026 The Bitcoin Core developers
--- Unit Test Suite (80 test cases including randomx_v2_mid_world_test) ---
Running 80 test cases...
test/pow_tests.cpp(1038): [1;31;49merror: in "pow_tests/randomx_v2_mid_world_test": check CheckProofOfWorkRandomX(block, chainParams->GetConsensus(), POW_VERIFY_MINING, &rx_hash_v1) has failed[0;39;49m
test/pow_tests.cpp(1040): [1;31;49merror: in "pow_tests/randomx_v2_mid_world_test": check CheckProofOfWorkRandomX(block, chainParams->GetConsensus(), POW_VERIFY_FULL) has failed[0;39;49m
test/pow_tests.cpp(1046): [1;31;49merror: in "pow_tests/randomx_v2_mid_world_test": check CheckProofOfWorkRandomX(block_v2, chainParams->GetConsensus(), POW_VERIFY_MINING, &rx_hash_v2) has failed[0;39;49m
test/pow_tests.cpp(1048): [1;31;49merror: in "pow_tests/randomx_v2_mid_world_test": check CheckProofOfWorkRandomX(block_v2, chainParams->GetConsensus(), POW_VERIFY_FULL) has failed[0;39;49m
test/pow_tests.cpp(1049): [1;31;49merror: in "pow_tests/randomx_v2_mid_world_test": check rx_hash_v1 != rx_hash_v2 has failed[0;39;49m

[1;31;49m*** 5 failures are detected in the test module "Bitcoin Core Test Suite"
[0;39;49mbash: line 86: ./build/libexec/test_scash: No such file or directory
--- Live Scash Regtest Check (0.1 sat/vB minrelaytxfee, OP_RETURN blocked, & legacy getwalletinfo fields) ---
Scash starting
[getmempoolinfo]
  "loaded": true,
  "minrelaytxfee": 0.00000100,
  "incrementalrelayfee": 0.00000100,
  "maxdatacarriersize": 0
[createwallet & getwalletinfo legacy fields]
  "walletname": "testwallet",
[getmininginfo]
  "chain": "scashregtest",
  "pow_algo": "randomx",
  "warnings": ""
Scash stopping
```
