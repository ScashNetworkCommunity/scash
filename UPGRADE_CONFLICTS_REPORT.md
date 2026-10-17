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
--
916-/usr/bin/ld: ./src/./src/arith_uint256.cpp:161:(.text._ZN9base_uintILj512EE6SetHexERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE[_ZN9base_uintILj512EE6SetHexERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE]+0x37): undefined reference to `base_blob<512u>::SetHex(char const*)'
917-/usr/bin/ld: ./src/./src/arith_uint256.cpp:161:(.text._ZN9base_uintILj512EE6SetHexEPKc[_ZN9base_uintILj512EE6SetHexEPKc]+0x34): undefined reference to `base_blob<512u>::SetHex(char const*)'
918:clang++: error: linker command failed with exit code 1 (use -v to see invocation)
919-make[2]: *** [Makefile:7371: bitcoin-util] Error 1
920-  CXX      init/qt_bitcoin_qt-bitcoin-qt.o
--
1049-/usr/bin/ld: ./src/./src/arith_uint256.cpp:161:(.text._ZN9base_uintILj512EE6SetHexERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE[_ZN9base_uintILj512EE6SetHexERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE]+0x37): undefined reference to `base_blob<512u>::SetHex(char const*)'
1050-/usr/bin/ld: ./src/./src/arith_uint256.cpp:161:(.text._ZN9base_uintILj512EE6SetHexEPKc[_ZN9base_uintILj512EE6SetHexEPKc]+0x34): undefined reference to `base_blob<512u>::SetHex(char const*)'
1051:clang++: error: linker command failed with exit code 1 (use -v to see invocation)
1052-make[2]: *** [Makefile:7383: bitcoind] Error 1
1053-/usr/bin/ld: warning: jit_compiler_x86_static.S.o: missing .note.GNU-stack section implies executable stack
--
1061-/usr/bin/ld: ./src/./src/arith_uint256.cpp:161:(.text._ZN9base_uintILj512EE6SetHexERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE[_ZN9base_uintILj512EE6SetHexERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE]+0x37): undefined reference to `base_blob<512u>::SetHex(char const*)'
1062-/usr/bin/ld: ./src/./src/arith_uint256.cpp:161:(.text._ZN9base_uintILj512EE6SetHexEPKc[_ZN9base_uintILj512EE6SetHexEPKc]+0x34): undefined reference to `base_blob<512u>::SetHex(char const*)'
1063:clang++: error: linker command failed with exit code 1 (use -v to see invocation)
1064-make[2]: *** [Makefile:7716: test/fuzz/fuzz] Error 1
1065-  AR       qt/libbitcoinqt.a
--
1086-/usr/bin/ld: ./src/./src/arith_uint256.cpp:161:(.text._ZN9base_uintILj512EE6SetHexERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE[_ZN9base_uintILj512EE6SetHexERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE]+0x37): undefined reference to `base_blob<512u>::SetHex(char const*)'
1087-/usr/bin/ld: ./src/./src/arith_uint256.cpp:161:(.text._ZN9base_uintILj512EE6SetHexEPKc[_ZN9base_uintILj512EE6SetHexEPKc]+0x34): undefined reference to `base_blob<512u>::SetHex(char const*)'
1088:clang++: error: linker command failed with exit code 1 (use -v to see invocation)
1089-make[2]: *** [Makefile:7397: qt/bitcoin-qt] Error 1
1090:clang++: error: linker command failed with exit code 1 (use -v to see invocation)
1091-make[2]: *** [Makefile:7440: qt/test/test_bitcoin-qt] Error 1
1092-make[2]: Target 'all-am' not remade because of errors.
--- Tail of build.log ---
/usr/bin/ld: NOTE: This behaviour is deprecated and will be removed in a future version of the linker
/usr/bin/ld: libbitcoin_consensus.a(libbitcoin_consensus_a-arith_uint256.o): in function `base_uint<256u>::SetHex(char const*)':
./src/./src/arith_uint256.cpp:161:(.text._ZN9base_uintILj256EEC2ERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE[_ZN9base_uintILj256EEC5ERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE]+0x2d): undefined reference to `base_blob<256u>::SetHex(char const*)'
/usr/bin/ld: ./src/./src/arith_uint256.cpp:161:(.text._ZN9base_uintILj256EE6SetHexERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE[_ZN9base_uintILj256EE6SetHexERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE]+0x2d): undefined reference to `base_blob<256u>::SetHex(char const*)'
/usr/bin/ld: ./src/./src/arith_uint256.cpp:161:(.text._ZN9base_uintILj256EE6SetHexEPKc[_ZN9base_uintILj256EE6SetHexEPKc]+0x2a): undefined reference to `base_blob<256u>::SetHex(char const*)'
/usr/bin/ld: libbitcoin_consensus.a(libbitcoin_consensus_a-arith_uint256.o): in function `base_uint<512u>::SetHex(char const*)':
./src/./src/arith_uint256.cpp:161:(.text._ZN9base_uintILj512EEC2ERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE[_ZN9base_uintILj512EEC5ERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE]+0x37): undefined reference to `base_blob<512u>::SetHex(char const*)'
/usr/bin/ld: ./src/./src/arith_uint256.cpp:161:(.text._ZN9base_uintILj512EE6SetHexERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE[_ZN9base_uintILj512EE6SetHexERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE]+0x37): undefined reference to `base_blob<512u>::SetHex(char const*)'
/usr/bin/ld: ./src/./src/arith_uint256.cpp:161:(.text._ZN9base_uintILj512EE6SetHexEPKc[_ZN9base_uintILj512EE6SetHexEPKc]+0x34): undefined reference to `base_blob<512u>::SetHex(char const*)'
/usr/bin/ld: libbitcoin_consensus.a(libbitcoin_consensus_a-arith_uint256.o): in function `base_uint<256u>::SetHex(char const*)':
./src/./src/arith_uint256.cpp:161:(.text._ZN9base_uintILj256EEC2ERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE[_ZN9base_uintILj256EEC5ERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE]+0x2d): undefined reference to `base_blob<256u>::SetHex(char const*)'
/usr/bin/ld: ./src/./src/arith_uint256.cpp:161:(.text._ZN9base_uintILj256EE6SetHexERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE[_ZN9base_uintILj256EE6SetHexERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE]+0x2d): undefined reference to `base_blob<256u>::SetHex(char const*)'
/usr/bin/ld: ./src/./src/arith_uint256.cpp:161:(.text._ZN9base_uintILj256EE6SetHexEPKc[_ZN9base_uintILj256EE6SetHexEPKc]+0x2a): undefined reference to `base_blob<256u>::SetHex(char const*)'
/usr/bin/ld: libbitcoin_consensus.a(libbitcoin_consensus_a-arith_uint256.o): in function `base_uint<512u>::SetHex(char const*)':
./src/./src/arith_uint256.cpp:161:(.text._ZN9base_uintILj512EEC2ERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE[_ZN9base_uintILj512EEC5ERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE]+0x37): undefined reference to `base_blob<512u>::SetHex(char const*)'
/usr/bin/ld: ./src/./src/arith_uint256.cpp:161:(.text._ZN9base_uintILj512EE6SetHexERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE[_ZN9base_uintILj512EE6SetHexERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE]+0x37): undefined reference to `base_blob<512u>::SetHex(char const*)'
/usr/bin/ld: ./src/./src/arith_uint256.cpp:161:(.text._ZN9base_uintILj512EE6SetHexEPKc[_ZN9base_uintILj512EE6SetHexEPKc]+0x34): undefined reference to `base_blob<512u>::SetHex(char const*)'
clang++: error: linker command failed with exit code 1 (use -v to see invocation)
make[2]: *** [Makefile:7397: qt/bitcoin-qt] Error 1
clang++: error: linker command failed with exit code 1 (use -v to see invocation)
make[2]: *** [Makefile:7440: qt/test/test_bitcoin-qt] Error 1
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
