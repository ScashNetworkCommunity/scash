# Scash 3.0.0 (Mid-World) — Bitcoin Core v30.4 + RandomX v2 Build & Test Report

- **Branch**: `scash-30.4-Mid-World`
- **Target Core Release**: `v30.4` (Scash 3.0.0 "Mid-World")
- **RandomX Branch**: `ScashNetworkCommunity/RandomX@scash-v2` (`RANDOMX_PROGRAM_SIZE_V2=384`, `RANDOMX_FLAG_V2=128`)

## 1. CMake Configure Summary (exit code: 0)
```text
  fuzz binary ......................... OFF

Cross compiling ....................... FALSE
C++ compiler .......................... Clang 18.1.3, /usr/lib/ccache/clang++
CMAKE_BUILD_TYPE ...................... RelWithDebInfo
Preprocessor defined macros ........... 
C++ compiler flags .................... -O2 -g -std=c++20 -fPIC -fmacro-prefix-map=/home/runner/work/scash/scash/src=. -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3 -Wstack-protector -fstack-protector-all -fcf-protection=full -fstack-clash-protection -Wall -Wextra -Wgnu -Wformat -Wformat-security -Wvla -Wshadow-field -Wthread-safety -Wloop-analysis -Wredundant-decls -Wunused-member-function -Wdate-time -Wconditional-uninitialized -Woverloaded-virtual -Wsuggest-override -Wimplicit-fallthrough -Wunreachable-code -Wdocumentation -Wself-assign -Wundef -Wno-unused-parameter
Linker flags .......................... -O2 -g -fstack-protector-all -fcf-protection=full -fstack-clash-protection -Wl,-z,relro -Wl,-z,now -Wl,-z,separate-code -fPIE -pie

NOTE: The summary above may not exactly match the final applied build flags
      if any additional CMAKE_* or environment variables have been modified.
      To see the exact flags applied, build with the --verbose option.

Treat compiler warnings as errors ..... OFF
Use ccache for compiling .............. ON


-- Configuring done (22.8s)
-- Generating done (0.3s)
-- Build files have been written to: /home/runner/work/scash/scash/build
```

## 2. Build & Unit Test Outcome (exit code: 2)
COMPILER / LINKER ERRORS:
```text
5-[  1%] Built target generate_build_info
6-[  2%] Building C object src/secp256k1/src/CMakeFiles/secp256k1_precomputed.dir/precomputed_ecmult_gen.c.o
7-[  2%] Building CXX object src/univalue/CMakeFiles/univalue.dir/lib/univalue_get.cpp.o
8:/home/runner/work/scash/scash/src/arith_uint256.cpp:161:7: error: no member named 'SetHexDeprecated' in 'base_blob<256>'
9-  161 |     b.SetHexDeprecated(psz);
10-      |     ~ ^
11-/home/runner/work/scash/scash/src/arith_uint256.cpp:195:16: note: in instantiation of member function 'base_uint<256>::SetHex' requested here
12-  195 | template class base_uint<256>;
13-      |                ^
14:/home/runner/work/scash/scash/src/arith_uint256.cpp:161:7: error: no member named 'SetHexDeprecated' in 'base_blob<512>'
15-  161 |     b.SetHexDeprecated(psz);
16-      |     ~ ^
17-/home/runner/work/scash/scash/src/arith_uint256.cpp:259:16: note: in instantiation of member function 'base_uint<512>::SetHex' requested here
--
90-[ 10%] Linking CXX static library ../../lib/libbitcoin_crypto.a
91-[ 10%] Built target bitcoin_crypto
92-[ 10%] Building CXX object src/CMakeFiles/minisketch.dir/minisketch/src/minisketch.cpp.o
93:/home/runner/work/scash/scash/src/uint256.cpp:44:31: error: explicit instantiation of 'SetHexDeprecated' does not refer to a function template, variable template, member function, member class, or static data member
94-   44 | template void base_blob<512>::SetHexDeprecated(std::string_view);
95-      |                               ^
96-1 error generated.
--
453-[ 54%] Building CXX object src/CMakeFiles/bitcoin_node.dir/node/blockstorage.cpp.o
454-[ 54%] Building CXX object src/wallet/CMakeFiles/bitcoin_wallet.dir/rpc/backup.cpp.o
455-[ 54%] Building CXX object src/CMakeFiles/bitcoin_common.dir/kernel/chainparams.cpp.o
456:/home/runner/work/scash/scash/src/kernel/chainparams.cpp:699:19: error: no member named 'nRuleChangeActivationThreshold' in 'Consensus::Params'
457-  699 |         consensus.nRuleChangeActivationThreshold = 1815; // 90% of 2016
458-      |         ~~~~~~~~~ ^
459:/home/runner/work/scash/scash/src/kernel/chainparams.cpp:700:19: error: no member named 'nMinerConfirmationWindow' in 'Consensus::Params'
460-  700 |         consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
461-      |         ~~~~~~~~~ ^
462:/home/runner/work/scash/scash/src/kernel/chainparams.cpp:765:9: error: use of undeclared identifier 'checkpointData'
463-  765 |         checkpointData = {
464-      |         ^
465:/home/runner/work/scash/scash/src/kernel/chainparams.cpp:808:19: error: no member named 'nRuleChangeActivationThreshold' in 'Consensus::Params'
466-  808 |         consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
467-      |         ~~~~~~~~~ ^
468:/home/runner/work/scash/scash/src/kernel/chainparams.cpp:809:19: error: no member named 'nMinerConfirmationWindow' in 'Consensus::Params'
469-  809 |         consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
470-      |         ~~~~~~~~~ ^
471:/home/runner/work/scash/scash/src/kernel/chainparams.cpp:869:9: error: use of undeclared identifier 'checkpointData'
472-  869 |         checkpointData = {
473-      |         ^
474:/home/runner/work/scash/scash/src/kernel/chainparams.cpp:908:19: error: no member named 'nRuleChangeActivationThreshold' in 'Consensus::Params'
475-  908 |         consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
476-      |         ~~~~~~~~~ ^
477:/home/runner/work/scash/scash/src/kernel/chainparams.cpp:909:19: error: no member named 'nMinerConfirmationWindow' in 'Consensus::Params'
478-  909 |         consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
479-      |         ~~~~~~~~~ ^
480:/home/runner/work/scash/scash/src/kernel/chainparams.cpp:974:9: error: use of undeclared identifier 'checkpointData'
481-  974 |         checkpointData = {
482-      |         ^
483-9 errors generated.
--
624-[ 68%] Building CXX object src/CMakeFiles/bitcoin_node.dir/rpc/server_util.cpp.o
625-[ 68%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/networkstyle.cpp.o
626-[ 69%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/notificator.cpp.o
627:/home/runner/work/scash/scash/src/rpc/rawtransaction.cpp:456:33: error: no matching function for call to 'ConstructTransaction'
628-  456 |     CMutableTransaction rawTx = ConstructTransaction(request.params[0], request.params[1], request.params[2], rbf);
629-      |                                 ^~~~~~~~~~~~~~~~~~~~
630-/home/runner/work/scash/scash/src/rpc/rawtransaction_util.h:56:21: note: candidate function not viable: requires 5 arguments, but 4 were provided
631-   56 | CMutableTransaction ConstructTransaction(const UniValue& inputs_in, const UniValue& outputs_in, const UniValue& locktime, std::optional<bool> rbf, const uint32_t version);
632-      |                     ^                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
633:/home/runner/work/scash/scash/src/rpc/rawtransaction.cpp:457:25: error: redefinition of 'rawTx'
634-  457 |     CMutableTransaction rawTx = ConstructTransaction(request.params[0], request.params[1], request.params[2], rbf, self.Arg<uint32_t>("version"));
635-      |                         ^
636-/home/runner/work/scash/scash/src/rpc/rawtransaction.cpp:456:25: note: previous definition is here
--- Tail of cmake_build.log ---
[ 72%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/utilitydialog.cpp.o
[ 72%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/addressbookpage.cpp.o
[ 72%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/addresstablemodel.cpp.o
[ 72%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/askpassphrasedialog.cpp.o
[ 73%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/coincontroldialog.cpp.o
gmake[2]: Target 'src/CMakeFiles/bitcoin_node.dir/build' not remade because of errors.
gmake[1]: *** [CMakeFiles/Makefile2:874: src/CMakeFiles/bitcoin_node.dir/all] Error 2
[ 73%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/coincontroltreewidget.cpp.o
[ 73%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/createwalletdialog.cpp.o
[ 73%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/editaddressdialog.cpp.o
[ 73%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/openuridialog.cpp.o
[ 73%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/overviewpage.cpp.o
[ 73%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/paymentserver.cpp.o
[ 73%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/psbtoperationsdialog.cpp.o
[ 74%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/qrimagewidget.cpp.o
[ 74%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/receivecoinsdialog.cpp.o
[ 74%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/receiverequestdialog.cpp.o
[ 74%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/recentrequeststablemodel.cpp.o
[ 74%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/sendcoinsdialog.cpp.o
[ 74%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/sendcoinsentry.cpp.o
[ 74%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/signverifymessagedialog.cpp.o
[ 75%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/transactiondesc.cpp.o
[ 75%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/transactiondescdialog.cpp.o
[ 75%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/transactionfilterproxy.cpp.o
[ 75%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/transactionoverviewwidget.cpp.o
[ 75%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/transactionrecord.cpp.o
[ 75%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/transactiontablemodel.cpp.o
[ 75%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/transactionview.cpp.o
[ 75%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/walletcontroller.cpp.o
[ 76%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/walletframe.cpp.o
[ 76%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/walletmodel.cpp.o
[ 76%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/walletmodeltransaction.cpp.o
[ 76%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/walletview.cpp.o
[ 76%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/bitcoinqt_autogen/EWIEGA46WW/qrc_bitcoin.cpp.o
[ 76%] Building CXX object src/qt/CMakeFiles/bitcoinqt.dir/bitcoinqt_autogen/EJRQKI7XPS/qrc_bitcoin_locale.cpp.o
[ 76%] Linking CXX static library ../../lib/libbitcoinqt.a
[ 76%] Built target bitcoinqt
gmake[1]: Target 'all' not remade because of errors.
gmake: *** [Makefile:146: all] Error 2
gmake: Target 'default_target' not remade because of errors.
```
