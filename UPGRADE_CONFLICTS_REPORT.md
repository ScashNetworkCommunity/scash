# Scash v28.4 Build, Unit Test & Regtest Verification Report

## 1. Compiler & Configure Summary (exit code: 0)
```text
Ubuntu clang version 18.1.3 (1ubuntu1)
Target: x86_64-pc-linux-gnu
-rw-r--r-- 1 root root  13K Oct 17 20:03 /usr/local/include/randomx.h
-rw-r--r-- 1 root root 704K Oct 17 20:03 /usr/local/lib/librandomx.a
```

## 2. Build Outcome
**BUILD SUCCEEDED (bitcoind/scashd + bitcoin-qt/scash-qt + bitcoin-cli/scash-cli + test_bitcoin/test_scash)!**
```text
Scash version v2.0.0-narnia-core-28.4.0
Copyright (C) 2009-2026 The Bitcoin Core developers
```

## 3. Unit Test Suite Results
```text
Running 79 test cases...
test/rpc_tests.cpp(560): [1;31;49merror: in "rpc_tests/help_example": check HelpExampleRpcNamed("test", args) == "> curl --user myusername --data-binary '{\"jsonrpc\": \"1.0\", \"id\": \"curltest\", \"method\": \"test\", \"params\": {\"foo\":\"bar\",\"b\":true,\"n\":1}}' -H 'content-type: text/plain;' http://127.0.0.1:8332/\n" has failed [> curl --user myusername --data-binary '{"jsonrpc": "2.0", "id": "curltest", "method": "test", "params": {"foo":"bar","b":true,"n":1}}' -H 'content-type: application/json' http://127.0.0.1:8332/
 != > curl --user myusername --data-binary '{"jsonrpc": "1.0", "id": "curltest", "method": "test", "params": {"foo":"bar","b":true,"n":1}}' -H 'content-type: text/plain;' http://127.0.0.1:8332/
][0;39;49m
test/rpc_tests.cpp(577): [1;31;49merror: in "rpc_tests/help_example": check HelpExampleRpcNamed("test", {{"name", obj_value}}) == "> curl --user myusername --data-binary '{\"jsonrpc\": \"1.0\", \"id\": \"curltest\", \"method\": \"test\", \"params\": {\"name\":{\"foo\":\"bar\",\"b\":false,\"n\":1}}}' -H 'content-type: text/plain;' http://127.0.0.1:8332/\n" has failed [> curl --user myusername --data-binary '{"jsonrpc": "2.0", "id": "curltest", "method": "test", "params": {"name":{"foo":"bar","b":false,"n":1}}}' -H 'content-type: application/json' http://127.0.0.1:8332/
 != > curl --user myusername --data-binary '{"jsonrpc": "1.0", "id": "curltest", "method": "test", "params": {"name":{"foo":"bar","b":false,"n":1}}}' -H 'content-type: text/plain;' http://127.0.0.1:8332/
][0;39;49m
test/rpc_tests.cpp(587): [1;31;49merror: in "rpc_tests/help_example": check HelpExampleRpcNamed("test", {{"name", arr_value}}) == "> curl --user myusername --data-binary '{\"jsonrpc\": \"1.0\", \"id\": \"curltest\", \"method\": \"test\", \"params\": {\"name\":[\"bar\",false,1]}}' -H 'content-type: text/plain;' http://127.0.0.1:8332/\n" has failed [> curl --user myusername --data-binary '{"jsonrpc": "2.0", "id": "curltest", "method": "test", "params": {"name":["bar",false,1]}}' -H 'content-type: application/json' http://127.0.0.1:8332/
 != > curl --user myusername --data-binary '{"jsonrpc": "1.0", "id": "curltest", "method": "test", "params": {"name":["bar",false,1]}}' -H 'content-type: text/plain;' http://127.0.0.1:8332/
][0;39;49m

[1;31;49m*** 3 failures are detected in the test module "Bitcoin Core Test Suite"
[0;39;49m```

## 4. Live Regtest Verification of Scash v28.4 Policy Decisions
```text
Scash starting
--- Decision #1: Blockfile XOR Obfuscation (xor.dat) ---
--- Decision #2: Mempool Info (fullrbf = false) ---
  "loaded": true,
  "maxmempool": 300000000,
  "fullrbf": false
--- Decision #3: RPC warnings single-string format ---
  "chain": "regtest",
  "warnings": ""
}
Scash stopping
```
