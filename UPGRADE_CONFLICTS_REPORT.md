# Scash v28.4 Build & Unit Test Report (Ubuntu 24.04 + Clang 18 + Qt5)

## 1. Compiler & Configure Summary (exit code: 0)
```text
Ubuntu clang version 18.1.3 (1ubuntu1)
Target: x86_64-pc-linux-gnu
```

## 2. Build & Test Outcome
COMPILER / BUILD LOG OUTPUT:
```text
571-  CXX      test/test_bitcoin-argsman_tests.o
572-  CXX      test/test_bitcoin-arith_uint256_tests.o
573:test/arith_uint256_tests.cpp:285:20: error: use of undeclared identifier 'arith_uint256S'
574-  285 |     BOOST_CHECK_LT(arith_uint256S("0000000000000000000000000000000000000000000000000000000000000001"),
575-      |                    ^
576:test/arith_uint256_tests.cpp:286:20: error: use of undeclared identifier 'arith_uint256S'
577-  286 |                    arith_uint256S("1000000000000000000000000000000000000000000000000000000000000000"));
578-      |                    ^
--- Tail of build.log ---
  CXX      qt/libbitcoinqt_a-moc_walletmodel.o
  CXX      qt/libbitcoinqt_a-moc_walletview.o
  CXX      qt/libbitcoinqt_a-qrc_bitcoin.o
  CXX      qt/libbitcoinqt_a-qrc_bitcoin_locale.o
  AR       libtest_fuzz.a
  CXXLD    univalue/test/object
  CXXLD    univalue/test/unitester
  CXXLD    bitcoind
  CXXLD    bitcoin-cli
/usr/bin/ld: warning: jit_compiler_x86_static.S.o: missing .note.GNU-stack section implies executable stack
/usr/bin/ld: NOTE: This behaviour is deprecated and will be removed in a future version of the linker
  CXXLD    test/fuzz/fuzz
/usr/bin/ld: warning: jit_compiler_x86_static.S.o: missing .note.GNU-stack section implies executable stack
/usr/bin/ld: NOTE: This behaviour is deprecated and will be removed in a future version of the linker
  AR       qt/libbitcoinqt.a
  CXXLD    qt/bitcoin-qt
  CXXLD    qt/test/test_bitcoin-qt
/usr/bin/ld: warning: jit_compiler_x86_static.S.o: missing .note.GNU-stack section implies executable stack
/usr/bin/ld: NOTE: This behaviour is deprecated and will be removed in a future version of the linker
/usr/bin/ld: warning: jit_compiler_x86_static.S.o: missing .note.GNU-stack section implies executable stack
/usr/bin/ld: NOTE: This behaviour is deprecated and will be removed in a future version of the linker
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
