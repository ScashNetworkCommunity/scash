# Scash v28.4 Build & Unit Test Report

## 1. Building RandomX & Core Dependencies (NO_QT=1)
    CMAKE_EXPORT_NO_PACKAGE_REGISTRY


-- Build files have been written to: /home/runner/work/scash/scash/depends/work/build/x86_64-pc-linux-gnu/libnatpmp/f2433bec24ca3d3f22a8a7840728a3ac177f94ba-db875245a19/build
Building libnatpmp...
make[1]: Entering directory '/home/runner/work/scash/scash/depends/work/build/x86_64-pc-linux-gnu/libnatpmp/f2433bec24ca3d3f22a8a7840728a3ac177f94ba-db875245a19/build'
make[2]: Entering directory '/home/runner/work/scash/scash/depends/work/build/x86_64-pc-linux-gnu/libnatpmp/f2433bec24ca3d3f22a8a7840728a3ac177f94ba-db875245a19/build'
make[3]: Entering directory '/home/runner/work/scash/scash/depends/work/build/x86_64-pc-linux-gnu/libnatpmp/f2433bec24ca3d3f22a8a7840728a3ac177f94ba-db875245a19/build'
make[4]: Entering directory '/home/runner/work/scash/scash/depends/work/build/x86_64-pc-linux-gnu/libnatpmp/f2433bec24ca3d3f22a8a7840728a3ac177f94ba-db875245a19/build'
make[4]: Leaving directory '/home/runner/work/scash/scash/depends/work/build/x86_64-pc-linux-gnu/libnatpmp/f2433bec24ca3d3f22a8a7840728a3ac177f94ba-db875245a19/build'
make[4]: Entering directory '/home/runner/work/scash/scash/depends/work/build/x86_64-pc-linux-gnu/libnatpmp/f2433bec24ca3d3f22a8a7840728a3ac177f94ba-db875245a19/build'
[ 33%] Building C object CMakeFiles/natpmp.dir/natpmp.c.o
[ 66%] Building C object CMakeFiles/natpmp.dir/getgateway.c.o
[100%] Linking C static library libnatpmp.a
make[4]: Leaving directory '/home/runner/work/scash/scash/depends/work/build/x86_64-pc-linux-gnu/libnatpmp/f2433bec24ca3d3f22a8a7840728a3ac177f94ba-db875245a19/build'
[100%] Built target natpmp
make[3]: Leaving directory '/home/runner/work/scash/scash/depends/work/build/x86_64-pc-linux-gnu/libnatpmp/f2433bec24ca3d3f22a8a7840728a3ac177f94ba-db875245a19/build'
make[2]: Leaving directory '/home/runner/work/scash/scash/depends/work/build/x86_64-pc-linux-gnu/libnatpmp/f2433bec24ca3d3f22a8a7840728a3ac177f94ba-db875245a19/build'
make[1]: Leaving directory '/home/runner/work/scash/scash/depends/work/build/x86_64-pc-linux-gnu/libnatpmp/f2433bec24ca3d3f22a8a7840728a3ac177f94ba-db875245a19/build'
Staging libnatpmp...
Postprocessing libnatpmp...
Caching libnatpmp...
copying packages: randomx boost libevent bdb sqlite miniupnpc libnatpmp
to: /home/runner/work/scash/scash/depends/x86_64-pc-linux-gnu
make: Leaving directory '/home/runner/work/scash/scash/depends'

## 2. Running autogen.sh & configure
libtoolize: putting auxiliary files in AC_CONFIG_AUX_DIR, 'build-aux'.
libtoolize: copying file 'build-aux/ltmain.sh'
libtoolize: putting macros in AC_CONFIG_MACRO_DIRS, 'build-aux/m4'.
libtoolize: copying file 'build-aux/m4/libtool.m4'
libtoolize: copying file 'build-aux/m4/ltoptions.m4'
libtoolize: copying file 'build-aux/m4/ltsugar.m4'
libtoolize: copying file 'build-aux/m4/ltversion.m4'
libtoolize: copying file 'build-aux/m4/lt~obsolete.m4'
configure.ac:140: installing 'build-aux/compile'
configure.ac:77: installing 'build-aux/config.guess'
configure.ac:77: installing 'build-aux/config.sub'
configure.ac:84: installing 'build-aux/install-sh'
configure.ac:84: installing 'build-aux/missing'
src/Makefile.am: installing 'build-aux/depcomp'
parallel-tests: installing 'build-aux/test-driver'
Options used to compile and link:
  external signer = yes
  multiprocess    = no
  with wallet     = yes
    with sqlite   = yes
    with bdb      = yes
  with gui / qt   = no
  with zmq        = no
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

  CC              = gcc -m64
  CFLAGS          = -pthread  -pipe -std=c11 -O2 
  CPPFLAGS        =   -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3  -DHAVE_BUILD_INFO -DPROVIDE_FUZZ_MAIN_FUNCTION -I/home/runner/work/scash/scash/depends/x86_64-pc-linux-gnu/include/ 
  CXX             = g++ -m64 -std=c++20
  CXXFLAGS        =  -g -O2 -fno-extended-identifiers -fstack-reuse=none   -Wstack-protector -fstack-protector-all -fcf-protection=full -fstack-clash-protection  -Wall -Wextra -Wformat -Wformat-security -Wvla -Wredundant-decls -Wdate-time -Wduplicated-branches -Wduplicated-cond -Wlogical-op -Woverloaded-virtual -Wsuggest-override -Wimplicit-fallthrough -Wunreachable-code -Wundef  -Wno-unused-parameter   -pipe -std=c++20 -O2 
  LDFLAGS         = -lpthread  -Wl,-z,relro -Wl,-z,now -Wl,-z,separate-code -pie   -L/home/runner/work/scash/scash/depends/x86_64-pc-linux-gnu/lib 
  AR              = ar
  ARFLAGS         = cr


## 3. Compiling scashd, scash-cli, and test_scash
Making all in src
make[1]: Entering directory '/home/runner/work/scash/scash/src'
make[2]: Entering directory '/home/runner/work/scash/scash/src'
  CXX      bitcoind-bitcoind.o
  CXX      libbitcoin_node_a-addrdb.o
  CXX      libbitcoin_node_a-addrman.o
  CXX      libbitcoin_node_a-banman.o
  CXX      libbitcoin_node_a-bip324.o
  CXX      libbitcoin_node_a-blockencodings.o
  CXX      libbitcoin_node_a-blockfilter.o
  CXX      libbitcoin_node_a-chain.o
  CXX      libbitcoin_node_a-dbwrapper.o
  CXX      libbitcoin_node_a-deploymentstatus.o
  CXX      libbitcoin_node_a-flatfile.o
  CXX      libbitcoin_node_a-headerssync.o
  CXX      libbitcoin_node_a-httprpc.o
  CXX      libbitcoin_node_a-httpserver.o
  CXX      libbitcoin_node_a-i2p.o
  CXX      libbitcoin_node_a-init.o
  CXX      libbitcoin_node_a-mapport.o
  CXX      libbitcoin_node_a-net.o
  CXX      libbitcoin_node_a-net_processing.o
  CXX      libbitcoin_node_a-netgroup.o
  CXX      libbitcoin_node_a-noui.o
  CXX      libbitcoin_node_a-pow.o
  CXX      libbitcoin_node_a-rest.o
  CXX      libbitcoin_node_a-signet.o
  CXX      libbitcoin_node_a-torcontrol.o
  CXX      libbitcoin_node_a-txdb.o
  CXX      libbitcoin_node_a-txmempool.o
  CXX      libbitcoin_node_a-txorphanage.o
  CXX      libbitcoin_node_a-txrequest.o
  CXX      libbitcoin_node_a-validation.o
validation.cpp: In member function ‘bool {anonymous}::MemPoolAccept::PolicyScriptChecks(const {anonymous}::MemPoolAccept::ATMPArgs&, {anonymous}::MemPoolAccept::Workspace&)’:
validation.cpp:1254:88: warning: enumerated and non-enumerated type in conditional expression [-Wextra]
 1254 |     const unsigned int scriptVerifyFlags = STANDARD_SCRIPT_VERIFY_FLAGS | (g_isRandomX ? SCRIPT_VERIFY_DISCOURAGE_ORDINALS : 0);
      |                                                                            ~~~~~~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
validation.cpp: In member function ‘bool Chainstate::ActivateBestChainStep(BlockValidationState&, CBlockIndex*, const std::shared_ptr<const CBlock>&, bool&, ConnectTrace&)’:
validation.cpp:3360:66: error: invalid initialization of reference of type ‘const bilingual_str&’ from expression of type ‘const std::__cxx11::basic_string<char>’
 3360 |                 FatalError(m_chainman.GetNotifications(), state, strMessage);
      |                                                                  ^~~~~~~~~~
validation.cpp:2243:97: note: in passing argument 3 of ‘bool FatalError(kernel::Notifications&, BlockValidationState&, const bilingual_str&)’
 2243 | bool FatalError(Notifications& notifications, BlockValidationState& state, const bilingual_str& message)
      |                                                                            ~~~~~~~~~~~~~~~~~~~~~^~~~~~~
  CXX      libbitcoin_node_a-validationinterface.o
  CXX      libbitcoin_node_a-versionbits.o
make[2]: *** [Makefile:11264: libbitcoin_node_a-validation.o] Error 1
make[2]: *** Waiting for unfinished jobs....
make[2]: Leaving directory '/home/runner/work/scash/scash/src'
make[1]: *** [Makefile:20325: all-recursive] Error 1
make[1]: Leaving directory '/home/runner/work/scash/scash/src'
make: *** [Makefile:759: all-recursive] Error 1
BUILD ERRORS DETECTED:
```text
36- 1254 |     const unsigned int scriptVerifyFlags = STANDARD_SCRIPT_VERIFY_FLAGS | (g_isRandomX ? SCRIPT_VERIFY_DISCOURAGE_ORDINALS : 0);
37-      |                                                                            ~~~~~~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
38-validation.cpp: In member function ‘bool Chainstate::ActivateBestChainStep(BlockValidationState&, CBlockIndex*, const std::shared_ptr<const CBlock>&, bool&, ConnectTrace&)’:
39:validation.cpp:3360:66: error: invalid initialization of reference of type ‘const bilingual_str&’ from expression of type ‘const std::__cxx11::basic_string<char>’
40- 3360 |                 FatalError(m_chainman.GetNotifications(), state, strMessage);
41-      |                                                                  ^~~~~~~~~~
42-validation.cpp:2243:97: note: in passing argument 3 of ‘bool FatalError(kernel::Notifications&, BlockValidationState&, const bilingual_str&)’
```
