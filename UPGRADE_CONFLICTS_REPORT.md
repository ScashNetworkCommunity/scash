# Scash v28.4 Build & Unit Test Report (Ubuntu 24.04 + Clang 18 + Qt5)

## 1. Compiler & Configure Summary (exit code: 0)
```text
Ubuntu clang version 18.1.3 (1ubuntu1)
Target: x86_64-pc-linux-gnu
  with gui / qt   = yes
    with qr       = yes
  with zmq        = yes
  with test       = yes
  with fuzz binary = yes
  with bench      = no
  with upnp       = yes
  with natpmp     = yes
  USDT tracing    = no
  sanitizers      = 
  debug enabled   = no
  werror          = no

  target os       = linux-gnu
  build os        = linux-gnu

  CC              = /usr/bin/ccache clang
  CFLAGS          = -pthread  -g -O2
  CPPFLAGS        =  -fmacro-prefix-map=$(abs_top_srcdir)=.  -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3  -DHAVE_BUILD_INFO -DPROVIDE_FUZZ_MAIN_FUNCTION -I/usr/local/include
  CXX             = /usr/bin/ccache clang++ -std=c++20
  CXXFLAGS        =  -g -O2  -fdebug-prefix-map=$(abs_top_srcdir)=.  -Wstack-protector -fstack-protector-all -fcf-protection=full -fstack-clash-protection  -Wall -Wextra -Wgnu -Wformat -Wformat-security -Wvla -Wshadow-field -Wthread-safety -Wloop-analysis -Wredundant-decls -Wunused-member-function -Wdate-time -Wconditional-uninitialized -Woverloaded-virtual -Wsuggest-override -Wimplicit-fallthrough -Wunreachable-code -Wdocumentation -Wself-assign -Wundef  -Wno-unused-parameter   
  LDFLAGS         = -lpthread  -Wl,-z,relro -Wl,-z,now -Wl,-z,separate-code -pie   -L/usr/local/lib
  AR              = /usr/bin/ar
  ARFLAGS         = cr

```

## 2. Build & Test Outcome
COMPILER / BUILD LOG OUTPUT:
```text
112-  CXX      libbitcoin_util_a-sync.o
113-  CXX      libbitcoin_consensus_a-arith_uint256.o
114:arith_uint256.cpp:161:7: error: no member named 'SetHex' in 'base_blob<256>'
115-  161 |     b.SetHex(psz);
116-      |     ~ ^
--
118-  195 | template class base_uint<256>;
119-      |                ^
120:arith_uint256.cpp:161:7: error: no member named 'SetHex' in 'base_blob<512>'
121-  161 |     b.SetHex(psz);
122-      |     ~ ^
--
137-  CXX      crypto/libbitcoin_crypto_sse41_la-sha256_sse41.lo
138-  CXX      crypto/libbitcoin_crypto_x86_shani_la-sha256_x86_shani.lo
139:uint256.cpp:70:31: error: explicit instantiation of 'SetHex' does not refer to a function template, variable template, member function, member class, or static data member
140-   70 | template void base_blob<512>::SetHex(const char*);
141-      |                               ^
142:uint256.cpp:71:31: error: explicit instantiation of 'SetHex' does not refer to a function template, variable template, member function, member class, or static data member
143-   71 | template void base_blob<512>::SetHex(const std::string&);
144-      |                               ^
--
411-  CXX      node/libbitcoin_node_a-chainstatemanager_args.o
412-  CXX      node/libbitcoin_node_a-coin.o
413:node/blockstorage.cpp:1065:16: error: use of undeclared identifier 'error'
414- 1065 |         return error("ReadBlockFromDisk: Errors in block header at %s", pos.ToString());
415-      |                ^
--
447-  CXX      rpc/libbitcoin_node_a-mempool.o
448-  CXX      rpc/libbitcoin_node_a-mining.o
449:rpc/blockchain.cpp:1312:19: error: unexpected namespace name 'node': expected expression
450- 1312 |                 (!node.args.IsArgNegated("-deprecatedrpc") && (node.args.GetArgs("-deprecatedrpc").empty() || IsDeprecatedRPCEnabled("warnings")) ?
451-      |                   ^
452:rpc/blockchain.cpp:1312:64: error: unexpected namespace name 'node': expected expression
453- 1312 |                 (!node.args.IsArgNegated("-deprecatedrpc") && (node.args.GetArgs("-deprecatedrpc").empty() || IsDeprecatedRPCEnabled("warnings")) ?
454-      |                                                                ^
455:rpc/blockchain.cpp:1358:94: error: member reference type 'ArgsManager *' is a pointer; did you mean to use '->'?
456- 1358 |     obj.pushKV("warnings", node::GetWarningsForRpc(*CHECK_NONFATAL(node.warnings), !node.args.IsArgNegated("-deprecatedrpc") && (node.args.GetArgs("-deprecatedrpc").empty() || IsDeprecatedRPCEnabled("warnings"))));
457-      |                                                                                     ~~~~~~~~~^
458-      |                                                                                              ->
459:rpc/blockchain.cpp:1358:139: error: member reference type 'ArgsManager *' is a pointer; did you mean to use '->'?
460- 1358 |     obj.pushKV("warnings", node::GetWarningsForRpc(*CHECK_NONFATAL(node.warnings), !node.args.IsArgNegated("-deprecatedrpc") && (node.args.GetArgs("-deprecatedrpc").empty() || IsDeprecatedRPCEnabled("warnings"))));
461-      |                                                                                                                                  ~~~~~~~~~^
--
465-  CXX      rpc/libbitcoin_node_a-net.o
466-  CXX      rpc/libbitcoin_node_a-node.o
467:rpc/mining.cpp:445:27: error: unexpected namespace name 'node': expected expression
468-  445 |                         (!node.args.IsArgNegated("-deprecatedrpc") && (node.args.GetArgs("-deprecatedrpc").empty() || IsDeprecatedRPCEnabled("warnings")) ?
469-      |                           ^
470:rpc/mining.cpp:445:72: error: unexpected namespace name 'node': expected expression
471-  445 |                         (!node.args.IsArgNegated("-deprecatedrpc") && (node.args.GetArgs("-deprecatedrpc").empty() || IsDeprecatedRPCEnabled("warnings")) ?
472-      |                                                                        ^
473:rpc/mining.cpp:474:94: error: member reference type 'ArgsManager *' is a pointer; did you mean to use '->'?
474-  474 |     obj.pushKV("warnings", node::GetWarningsForRpc(*CHECK_NONFATAL(node.warnings), !node.args.IsArgNegated("-deprecatedrpc") && (node.args.GetArgs("-deprecatedrpc").empty() || IsDeprecatedRPCEnabled("warnings"))));
475-      |                                                                                     ~~~~~~~~~^
476-      |                                                                                              ->
477:rpc/mining.cpp:474:94: error: incomplete definition of type 'ArgsManager'
478-  474 |     obj.pushKV("warnings", node::GetWarningsForRpc(*CHECK_NONFATAL(node.warnings), !node.args.IsArgNegated("-deprecatedrpc") && (node.args.GetArgs("-deprecatedrpc").empty() || IsDeprecatedRPCEnabled("warnings"))));
479-      |                                                                                     ~~~~~~~~~^
--
481-   13 | class ArgsManager;
482-      |       ^
483:rpc/mining.cpp:474:139: error: member reference type 'ArgsManager *' is a pointer; did you mean to use '->'?
484-  474 |     obj.pushKV("warnings", node::GetWarningsForRpc(*CHECK_NONFATAL(node.warnings), !node.args.IsArgNegated("-deprecatedrpc") && (node.args.GetArgs("-deprecatedrpc").empty() || IsDeprecatedRPCEnabled("warnings"))));
485-      |                                                                                                                                  ~~~~~~~~~^
486-      |                                                                                                                                           ->
487:rpc/mining.cpp:474:139: error: incomplete definition of type 'ArgsManager'
488-  474 |     obj.pushKV("warnings", node::GetWarningsForRpc(*CHECK_NONFATAL(node.warnings), !node.args.IsArgNegated("-deprecatedrpc") && (node.args.GetArgs("-deprecatedrpc").empty() || IsDeprecatedRPCEnabled("warnings"))));
489-      |                                                                                                                                  ~~~~~~~~~^
--
494-make[2]: *** [Makefile:11041: rpc/libbitcoin_node_a-mining.o] Error 1
495-  CXX      rpc/libbitcoin_node_a-output_script.o
496:rpc/net.cpp:665:27: error: unexpected namespace name 'node': expected expression
497-  665 |                         (!node.args.IsArgNegated("-deprecatedrpc") && (node.args.GetArgs("-deprecatedrpc").empty() || IsDeprecatedRPCEnabled("warnings")) ?
498-      |                           ^
499:rpc/net.cpp:665:72: error: unexpected namespace name 'node': expected expression
500-  665 |                         (!node.args.IsArgNegated("-deprecatedrpc") && (node.args.GetArgs("-deprecatedrpc").empty() || IsDeprecatedRPCEnabled("warnings")) ?
501-      |                                                                        ^
502:rpc/net.cpp:722:94: error: member reference type 'ArgsManager *' is a pointer; did you mean to use '->'?
503-  722 |     obj.pushKV("warnings", node::GetWarningsForRpc(*CHECK_NONFATAL(node.warnings), !node.args.IsArgNegated("-deprecatedrpc") && (node.args.GetArgs("-deprecatedrpc").empty() || IsDeprecatedRPCEnabled("warnings"))));
504-      |                                                                                     ~~~~~~~~~^
--- Tail of build.log ---
  CXX      qt/libbitcoinqt_a-moc_signverifymessagedialog.o
  CXX      qt/libbitcoinqt_a-moc_splashscreen.o
  CXX      qt/libbitcoinqt_a-moc_trafficgraphwidget.o
  CXX      qt/libbitcoinqt_a-moc_transactiondesc.o
  CXX      qt/libbitcoinqt_a-moc_transactiondescdialog.o
  CXX      qt/libbitcoinqt_a-moc_transactionfilterproxy.o
  CXX      qt/libbitcoinqt_a-moc_transactionoverviewwidget.o
  CXX      qt/libbitcoinqt_a-moc_transactiontablemodel.o
  CXX      qt/libbitcoinqt_a-moc_transactionview.o
  CXX      qt/libbitcoinqt_a-moc_utilitydialog.o
  CXX      qt/libbitcoinqt_a-moc_walletcontroller.o
  CXX      qt/libbitcoinqt_a-moc_walletframe.o
  CXX      qt/libbitcoinqt_a-moc_walletmodel.o
  CXX      qt/libbitcoinqt_a-moc_walletview.o
  CXX      qt/libbitcoinqt_a-qrc_bitcoin.o
  CXX      qt/libbitcoinqt_a-qrc_bitcoin_locale.o
  AR       libtest_fuzz.a
  CXXLD    univalue/test/object
  CXXLD    univalue/test/unitester
  CXXLD    bitcoin-cli
  AR       qt/libbitcoinqt.a
make[2]: Target 'all-am' not remade because of errors.
make[2]: Leaving directory '/home/runner/work/scash/scash/src'
make[1]: *** [Makefile:20326: all-recursive] Error 1
make[1]: Target 'all' not remade because of errors.
make[1]: Leaving directory '/home/runner/work/scash/scash/src'
Making all in doc/man
make[1]: Entering directory '/home/runner/work/scash/scash/doc/man'
make[1]: Nothing to be done for 'all'.
make[1]: Leaving directory '/home/runner/work/scash/scash/doc/man'
make[1]: Entering directory '/home/runner/work/scash/scash'
make[1]: Nothing to be done for 'all-am'.
make[1]: Leaving directory '/home/runner/work/scash/scash'
make: *** [Makefile:760: all-recursive] Error 1
make: Target 'all' not remade because of errors.
```
