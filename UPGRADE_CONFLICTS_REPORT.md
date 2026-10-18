# Scash v28.4 Build, Unit Test & Regtest Verification Report

## 1. Compiler & Configure Summary (exit code: 0)
```text
Ubuntu clang version 18.1.3 (1ubuntu1)
Target: x86_64-pc-linux-gnu
-rw-r--r-- 1 root root  13K Oct 18 00:06 /usr/local/include/randomx.h
-rw-r--r-- 1 root root 704K Oct 18 00:07 /usr/local/lib/librandomx.a
```

## 2. Build Outcome
**BUILD SUCCEEDED (bitcoind/scashd + bitcoin-qt/scash-qt + bitcoin-cli/scash-cli + test_bitcoin/test_scash)!**
```text
Scash version v2.0.0-narnia-core-28.4.0
Copyright (C) 2009-2026 The Bitcoin Core developers
```

## 3. Unit Test Suite Results (79 test cases)
```text
Running 79 test cases...
test/rpc_tests.cpp(235): [1;31;49merror: in "rpc_tests/rpc_format_monetary_values": check ValueFromAmount(100000000LL).write() == "2.00000000" has failed[0;39;49m
test/rpc_tests.cpp(241): [1;31;49merror: in "rpc_tests/rpc_format_monetary_values": check ValueFromAmount(-COIN).write() == "-2.00000000" has failed [-1.00000000 != -2.00000000][0;39;49m
test/rpc_tests.cpp(252): [1;31;49merror: in "rpc_tests/rpc_format_monetary_values": check ValueFromAmount(COIN).write() == "2.00000000" has failed [1.00000000 != 2.00000000][0;39;49m
test/rpc_tests.cpp(290): [1;31;49merror: in "rpc_tests/rpc_parse_monetary_values": check AmountFromValue(ValueFromString("2.00000000")) == 100000000LL has failed [200000000 != 100000000][0;39;49m

[1;31;49m*** 4 failures are detected in the test module "Bitcoin Core Test Suite"
[0;39;49m```

## 4. Live Scash Regtest Verification of v28.4 Policy Decisions
```text
Scash starting
--- Decision #1: Blockfile XOR Obfuscation (xor.dat) ---
-rw------- 1 runner runner 8 Oct 18 00:08 /tmp/tmp.71JCUKDn3y/scashregtest/blocks/xor.dat
--- Decision #2: Mempool Info (fullrbf = false) ---
  "loaded": true,
  "maxmempool": 300000000,
  "fullrbf": false
--- Decision #3: RPC warnings single-string format ---
  "chain": "scashregtest",
  "warnings": ""
Scash stopping
```
