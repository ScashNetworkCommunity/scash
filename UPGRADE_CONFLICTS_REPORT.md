# Scash v28.4 Build, Unit Test & Regtest Verification Report

## 1. Compiler & Configure Summary (exit code: 0)
```text
Ubuntu clang version 18.1.3 (1ubuntu1)
Target: x86_64-pc-linux-gnu
```

## 2. Build Outcome
COMPILER / BUILD LOG OUTPUT:
```text
908-/usr/bin/ld: ./src/./src/arith_uint256.cpp:161:(.text._ZN9base_uintILj512EE6SetHexERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE[_ZN9base_uintILj512EE6SetHexERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE]+0x37): undefined reference to `base_blob<512u>::SetHex(char const*)'
909-/usr/bin/ld: ./src/./src/arith_uint256.cpp:161:(.text._ZN9base_uintILj512EE6SetHexEPKc[_ZN9base_uintILj512EE6SetHexEPKc]+0x34): undefined reference to `base_blob<512u>::SetHex(char const*)'
910:clang++: error: linker command failed with exit code 1 (use -v to see invocation)
911-make[2]: *** [Makefile:7371: bitcoin-util] Error 1
912-  CXX      init/qt_bitcoin_qt-bitcoin-qt.o
--
1042-/usr/bin/ld: ./src/./src/arith_uint256.cpp:161:(.text._ZN9base_uintILj512EE6SetHexERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE[_ZN9base_uintILj512EE6SetHexERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE]+0x37): undefined reference to `base_blob<512u>::SetHex(char const*)'
1043-/usr/bin/ld: ./src/./src/arith_uint256.cpp:161:(.text._ZN9base_uintILj512EE6SetHexEPKc[_ZN9base_uintILj512EE6SetHexEPKc]+0x34): undefined reference to `base_blob<512u>::SetHex(char const*)'
1044:clang++: error: linker command failed with exit code 1 (use -v to see invocation)
1045-make[2]: *** [Makefile:7383: bitcoind] Error 1
1046-/usr/bin/ld: warning: jit_compiler_x86_static.S.o: missing .note.GNU-stack section implies executable stack
--
1056-/usr/bin/ld: ./src/./src/arith_uint256.cpp:161:(.text._ZN9base_uintILj512EE6SetHexERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE[_ZN9base_uintILj512EE6SetHexERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE]+0x37): undefined reference to `base_blob<512u>::SetHex(char const*)'
1057-/usr/bin/ld: ./src/./src/arith_uint256.cpp:161:(.text._ZN9base_uintILj512EE6SetHexEPKc[_ZN9base_uintILj512EE6SetHexEPKc]+0x34): undefined reference to `base_blob<512u>::SetHex(char const*)'
1058:clang++: error: linker command failed with exit code 1 (use -v to see invocation)
1059-make[2]: *** [Makefile:7716: test/fuzz/fuzz] Error 1
1060-/usr/bin/ld: libbitcoin_consensus.a(libbitcoin_consensus_a-arith_uint256.o): in function `base_uint<256u>::SetHex(char const*)':
--
1066-/usr/bin/ld: ./src/./src/arith_uint256.cpp:161:(.text._ZN9base_uintILj512EE6SetHexERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE[_ZN9base_uintILj512EE6SetHexERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE]+0x37): undefined reference to `base_blob<512u>::SetHex(char const*)'
1067-/usr/bin/ld: ./src/./src/arith_uint256.cpp:161:(.text._ZN9base_uintILj512EE6SetHexEPKc[_ZN9base_uintILj512EE6SetHexEPKc]+0x34): undefined reference to `base_blob<512u>::SetHex(char const*)'
1068:clang++: error: linker command failed with exit code 1 (use -v to see invocation)
1069-make[2]: *** [Makefile:7999: test/test_bitcoin] Error 1
1070-  AR       qt/libbitcoinqt.a
--
1091-/usr/bin/ld: ./src/./src/arith_uint256.cpp:161:(.text._ZN9base_uintILj512EE6SetHexERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE[_ZN9base_uintILj512EE6SetHexERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE]+0x37): undefined reference to `base_blob<512u>::SetHex(char const*)'
1092-/usr/bin/ld: ./src/./src/arith_uint256.cpp:161:(.text._ZN9base_uintILj512EE6SetHexEPKc[_ZN9base_uintILj512EE6SetHexEPKc]+0x34): undefined reference to `base_blob<512u>::SetHex(char const*)'
1093:clang++: error: linker command failed with exit code 1 (use -v to see invocation)
1094-make[2]: *** [Makefile:7397: qt/bitcoin-qt] Error 1
1095:clang++: error: linker command failed with exit code 1 (use -v to see invocation)
1096-make[2]: *** [Makefile:7440: qt/test/test_bitcoin-qt] Error 1
1097-make[2]: Target 'all-am' not remade because of errors.
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
