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


-- Configuring done (23.0s)
-- Generating done (0.3s)
-- Build files have been written to: /home/runner/work/scash/scash/build
```

## 2. Build & Unit Test Outcome (exit code: 2)
COMPILER / LINKER ERRORS:
```text
749-/home/runner/work/scash/scash/src/test/pow_tests.cpp:933:9: warning: unused variable 'i' [-Wunused-variable]
750-  933 |     int i=0;
751-      |         ^
752:/home/runner/work/scash/scash/src/test/pow_tests.cpp:1038:17: error: use of undeclared identifier 'CheckProofOfWorkScash'
753- 1038 |     BOOST_CHECK(CheckProofOfWorkScash(block, block.nBits, chainParams->GetConsensus(), POW_VERIFY_MINING, &rx_hash_v1));
754-      |                 ^
755:/home/runner/work/scash/scash/src/test/pow_tests.cpp:1040:17: error: use of undeclared identifier 'CheckProofOfWorkScash'; did you mean 'CheckProofOfWorkRandomX'?
756- 1040 |     BOOST_CHECK(CheckProofOfWorkScash(block, block.nBits, chainParams->GetConsensus(), POW_VERIFY_FULL));
757-      |                 ^~~~~~~~~~~~~~~~~~~~~
758-      |                 CheckProofOfWorkRandomX
--
768-/home/runner/work/scash/scash/src/pow.h:74:6: note: 'CheckProofOfWorkRandomX' declared here
769-   74 | bool CheckProofOfWorkRandomX(const CBlockHeader& block, const Consensus::Params& params, POWVerifyMode mode = POW_VERIFY_FULL, uint256 *outHash = nullptr);
770-      |      ^
771:/home/runner/work/scash/scash/src/test/pow_tests.cpp:1040:46: error: reference to type 'const Consensus::Params' could not bind to an lvalue of type 'uint32_t' (aka 'unsigned int')
772- 1040 |     BOOST_CHECK(CheckProofOfWorkScash(block, block.nBits, chainParams->GetConsensus(), POW_VERIFY_FULL));
773-      |                                              ^~~~~~~~~~~
774-/usr/include/boost/test/tools/old/interface.hpp:83:6: note: expanded from macro 'BOOST_CHECK'
--
783-/home/runner/work/scash/scash/src/pow.h:74:82: note: passing argument to parameter 'params' here
784-   74 | bool CheckProofOfWorkRandomX(const CBlockHeader& block, const Consensus::Params& params, POWVerifyMode mode = POW_VERIFY_FULL, uint256 *outHash = nullptr);
785-      |                                                                                  ^
786:/home/runner/work/scash/scash/src/test/pow_tests.cpp:1046:17: error: use of undeclared identifier 'CheckProofOfWorkScash'
787- 1046 |     BOOST_CHECK(CheckProofOfWorkScash(block_v2, block_v2.nBits, chainParams->GetConsensus(), POW_VERIFY_MINING, &rx_hash_v2));
788-      |                 ^
789:/home/runner/work/scash/scash/src/test/pow_tests.cpp:1048:17: error: use of undeclared identifier 'CheckProofOfWorkScash'; did you mean 'CheckProofOfWorkRandomX'?
790- 1048 |     BOOST_CHECK(CheckProofOfWorkScash(block_v2, block_v2.nBits, chainParams->GetConsensus(), POW_VERIFY_FULL));
791-      |                 ^~~~~~~~~~~~~~~~~~~~~
792-      |                 CheckProofOfWorkRandomX
--
802-/home/runner/work/scash/scash/src/pow.h:74:6: note: 'CheckProofOfWorkRandomX' declared here
803-   74 | bool CheckProofOfWorkRandomX(const CBlockHeader& block, const Consensus::Params& params, POWVerifyMode mode = POW_VERIFY_FULL, uint256 *outHash = nullptr);
804-      |      ^
805:/home/runner/work/scash/scash/src/test/pow_tests.cpp:1048:49: error: reference to type 'const Consensus::Params' could not bind to an lvalue of type 'uint32_t' (aka 'unsigned int')
806- 1048 |     BOOST_CHECK(CheckProofOfWorkScash(block_v2, block_v2.nBits, chainParams->GetConsensus(), POW_VERIFY_FULL));
807-      |                                                 ^~~~~~~~~~~~~~
808-/usr/include/boost/test/tools/old/interface.hpp:83:6: note: expanded from macro 'BOOST_CHECK'
--- Tail of cmake_build.log ---
[ 95%] Building CXX object src/test/CMakeFiles/test_bitcoin.dir/__/wallet/test/feebumper_tests.cpp.o
[ 95%] Building CXX object src/qt/CMakeFiles/bitcoin-qt.dir/bitcoin-qt_autogen/mocs_compilation.cpp.o
[ 95%] Building CXX object src/qt/CMakeFiles/bitcoin-qt.dir/main.cpp.o
[ 95%] Building CXX object src/qt/CMakeFiles/bitcoin-qt.dir/__/init/bitcoin-qt.cpp.o
[ 96%] Linking CXX executable ../../bin/scash-qt
/usr/bin/ld: warning: jit_compiler_x86_static.S.o: missing .note.GNU-stack section implies executable stack
/usr/bin/ld: NOTE: This behaviour is deprecated and will be removed in a future version of the linker
[ 96%] Built target bitcoin-qt
[ 96%] Building CXX object src/test/CMakeFiles/test_bitcoin.dir/__/wallet/test/group_outputs_tests.cpp.o
[ 96%] Building CXX object src/qt/test/CMakeFiles/test_bitcoin-qt.dir/test_bitcoin-qt_autogen/mocs_compilation.cpp.o
[ 96%] Building CXX object src/qt/test/CMakeFiles/test_bitcoin-qt.dir/apptests.cpp.o
[ 96%] Building CXX object src/qt/test/CMakeFiles/test_bitcoin-qt.dir/optiontests.cpp.o
[ 96%] Building CXX object src/test/CMakeFiles/test_bitcoin.dir/__/wallet/test/init_tests.cpp.o
[ 97%] Building CXX object src/test/CMakeFiles/test_bitcoin.dir/__/wallet/test/ismine_tests.cpp.o
[ 97%] Building CXX object src/qt/test/CMakeFiles/test_bitcoin-qt.dir/rpcnestedtests.cpp.o
[ 98%] Building CXX object src/qt/test/CMakeFiles/test_bitcoin-qt.dir/test_main.cpp.o
[ 98%] Building CXX object src/qt/test/CMakeFiles/test_bitcoin-qt.dir/uritests.cpp.o
[ 98%] Building CXX object src/qt/test/CMakeFiles/test_bitcoin-qt.dir/util.cpp.o
[ 98%] Building CXX object src/qt/test/CMakeFiles/test_bitcoin-qt.dir/__/__/init/bitcoin-qt.cpp.o
[ 98%] Building CXX object src/qt/test/CMakeFiles/test_bitcoin-qt.dir/addressbooktests.cpp.o
[ 98%] Building CXX object src/qt/test/CMakeFiles/test_bitcoin-qt.dir/wallettests.cpp.o
[ 98%] Building CXX object src/qt/test/CMakeFiles/test_bitcoin-qt.dir/__/__/wallet/test/wallet_test_fixture.cpp.o
[ 98%] Building CXX object src/test/CMakeFiles/test_bitcoin.dir/__/wallet/test/psbt_wallet_tests.cpp.o
[ 98%] Building CXX object src/test/CMakeFiles/test_bitcoin.dir/__/wallet/test/scriptpubkeyman_tests.cpp.o
[ 98%] Building CXX object src/test/CMakeFiles/test_bitcoin.dir/__/wallet/test/spend_tests.cpp.o
[ 99%] Linking CXX executable ../../../bin/test_scash-qt
/usr/bin/ld: warning: jit_compiler_x86_static.S.o: missing .note.GNU-stack section implies executable stack
/usr/bin/ld: NOTE: This behaviour is deprecated and will be removed in a future version of the linker
[ 99%] Built target test_bitcoin-qt
[ 99%] Building CXX object src/test/CMakeFiles/test_bitcoin.dir/__/wallet/test/wallet_crypto_tests.cpp.o
[ 99%] Building CXX object src/test/CMakeFiles/test_bitcoin.dir/__/wallet/test/wallet_rpc_tests.cpp.o
[ 99%] Building CXX object src/test/CMakeFiles/test_bitcoin.dir/__/wallet/test/wallet_tests.cpp.o
[100%] Building CXX object src/test/CMakeFiles/test_bitcoin.dir/__/wallet/test/wallet_transaction_tests.cpp.o
[100%] Building CXX object src/test/CMakeFiles/test_bitcoin.dir/__/wallet/test/walletdb_tests.cpp.o
[100%] Building CXX object src/test/CMakeFiles/test_bitcoin.dir/__/wallet/test/walletload_tests.cpp.o
gmake[2]: Target 'src/test/CMakeFiles/test_bitcoin.dir/build' not remade because of errors.
gmake[1]: *** [CMakeFiles/Makefile2:1956: src/test/CMakeFiles/test_bitcoin.dir/all] Error 2
gmake[1]: Target 'all' not remade because of errors.
gmake: *** [Makefile:146: all] Error 2
gmake: Target 'default_target' not remade because of errors.
```
