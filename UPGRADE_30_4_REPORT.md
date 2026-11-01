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
-- Generating done (0.2s)
-- Build files have been written to: /home/runner/work/scash/scash/build
```

## 2. Build & Unit Test Outcome (exit code: 2)
COMPILER / LINKER ERRORS:
```text
113-In file included from /home/runner/work/scash/scash/src/addrdb.cpp:11:
114-In file included from /home/runner/work/scash/scash/src/chainparams.h:9:
115-In file included from /home/runner/work/scash/scash/src/kernel/chainparams.h:10:
116:/home/runner/work/scash/scash/src/consensus/params.h:24:14: error: redefinition of 'nRuleChangeActivationThreshold'
117-   24 |     uint32_t nRuleChangeActivationThreshold{0};
118-      |              ^
119-/home/runner/work/scash/scash/src/consensus/params.h:19:14: note: previous definition is here
120-   19 |     uint32_t nRuleChangeActivationThreshold{0};
121-      |              ^
122:/home/runner/work/scash/scash/src/consensus/params.h:25:14: error: redefinition of 'nMinerConfirmationWindow'
123-   25 |     uint32_t nMinerConfirmationWindow{0};
124-      |              ^
125-/home/runner/work/scash/scash/src/consensus/params.h:20:14: note: previous definition is here
126-   20 |     uint32_t nMinerConfirmationWindow{0};
127-      |              ^
128:/home/runner/work/scash/scash/src/consensus/params.h:26:14: error: redefinition of 'nRandomXV2ActivationEpoch'
129-   26 |     uint32_t nRandomXV2ActivationEpoch{2970}; END
130-      |              ^
131-/home/runner/work/scash/scash/src/consensus/params.h:21:14: note: previous definition is here
132-   21 |     uint32_t nRandomXV2ActivationEpoch{2970};
133-      |              ^
134:/home/runner/work/scash/scash/src/consensus/params.h:26:47: error: unknown type name 'END'
135-   26 |     uint32_t nRandomXV2ActivationEpoch{2970}; END
136-      |                                               ^
137:/home/runner/work/scash/scash/src/consensus/params.h:28:1: error: expected unqualified-id
138-   28 | namespace Consensus {
139-      | ^
140:/home/runner/work/scash/scash/src/consensus/params.h:184:14: error: duplicate member 'nRuleChangeActivationThreshold'
141-  184 |     uint32_t nRuleChangeActivationThreshold{0};
142-      |              ^
143-/home/runner/work/scash/scash/src/consensus/params.h:167:14: note: previous declaration is here
144-  167 |     uint32_t nRuleChangeActivationThreshold{0};
145-      |              ^
146:/home/runner/work/scash/scash/src/consensus/params.h:185:14: error: duplicate member 'nMinerConfirmationWindow'
147-  185 |     uint32_t nMinerConfirmationWindow{0};
148-      |              ^
149-/home/runner/work/scash/scash/src/consensus/params.h:168:14: note: previous declaration is here
150-  168 |     uint32_t nMinerConfirmationWindow{0};
151-      |              ^
152:/home/runner/work/scash/scash/src/consensus/params.h:186:14: error: duplicate member 'nRandomXV2ActivationEpoch'
153-  186 |     uint32_t nRandomXV2ActivationEpoch{2970}; END
154-      |              ^
155-/home/runner/work/scash/scash/src/consensus/params.h:169:14: note: previous declaration is here
156-  169 |     uint32_t nRandomXV2ActivationEpoch{2970};
157-      |              ^
158:/home/runner/work/scash/scash/src/consensus/params.h:186:47: error: unknown type name 'END'
159-  186 |     uint32_t nRandomXV2ActivationEpoch{2970}; END
160-      |                                               ^
161:/home/runner/work/scash/scash/src/consensus/params.h:187:1: error: expected member name or ';' after declaration specifiers
162-  186 |     uint32_t nRandomXV2ActivationEpoch{2970}; END
163-      |                                               ~~~
164-  187 | };
--
232-In file included from /home/runner/work/scash/scash/src/bip324.cpp:7:
233-In file included from /home/runner/work/scash/scash/src/chainparams.h:9:
234-In file included from /home/runner/work/scash/scash/src/kernel/chainparams.h:10:
235:/home/runner/work/scash/scash/src/consensus/params.h:24:14: error: redefinition of 'nRuleChangeActivationThreshold'
236-   24 |     uint32_t nRuleChangeActivationThreshold{0};
237-      |              ^
238-/home/runner/work/scash/scash/src/consensus/params.h:19:14: note: previous definition is here
239-   19 |     uint32_t nRuleChangeActivationThreshold{0};
240-      |              ^
241:/home/runner/work/scash/scash/src/consensus/params.h:25:14: error: redefinition of 'nMinerConfirmationWindow'
242-   25 |     uint32_t nMinerConfirmationWindow{0};
243-      |              ^
244-/home/runner/work/scash/scash/src/consensus/params.h:20:14: note: previous definition is here
245-   20 |     uint32_t nMinerConfirmationWindow{0};
246-      |              ^
247:/home/runner/work/scash/scash/src/consensus/params.h:26:14: error: redefinition of 'nRandomXV2ActivationEpoch'
248-   26 |     uint32_t nRandomXV2ActivationEpoch{2970}; END
249-      |              ^
250-/home/runner/work/scash/scash/src/consensus/params.h:21:14: note: previous definition is here
251-   21 |     uint32_t nRandomXV2ActivationEpoch{2970};
252-      |              ^
253:/home/runner/work/scash/scash/src/consensus/params.h:26:47: error: unknown type name 'END'
254-   26 |     uint32_t nRandomXV2ActivationEpoch{2970}; END
255-      |                                               ^
256:/home/runner/work/scash/scash/src/consensus/params.h:28:1: error: expected unqualified-id
257-   28 | namespace Consensus {
258-      | ^
259:/home/runner/work/scash/scash/src/consensus/params.h:184:14: error: duplicate member 'nRuleChangeActivationThreshold'
260-  184 |     uint32_t nRuleChangeActivationThreshold{0};
261-      |              ^
262-/home/runner/work/scash/scash/src/consensus/params.h:167:14: note: previous declaration is here
263-  167 |     uint32_t nRuleChangeActivationThreshold{0};
264-      |              ^
265:/home/runner/work/scash/scash/src/consensus/params.h:185:14: error: duplicate member 'nMinerConfirmationWindow'
266-  185 |     uint32_t nMinerConfirmationWindow{0};
267-      |              ^
268-/home/runner/work/scash/scash/src/consensus/params.h:168:14: note: previous declaration is here
269-  168 |     uint32_t nMinerConfirmationWindow{0};
270-      |              ^
271:/home/runner/work/scash/scash/src/consensus/params.h:186:14: error: duplicate member 'nRandomXV2ActivationEpoch'
272-  186 |     uint32_t nRandomXV2ActivationEpoch{2970}; END
273-      |              ^
274-/home/runner/work/scash/scash/src/consensus/params.h:169:14: note: previous declaration is here
275-  169 |     uint32_t nRandomXV2ActivationEpoch{2970};
276-      |              ^
277:/home/runner/work/scash/scash/src/consensus/params.h:186:47: error: unknown type name 'END'
278-  186 |     uint32_t nRandomXV2ActivationEpoch{2970}; END
279-      |                                               ^
280:/home/runner/work/scash/scash/src/consensus/params.h:187:1: error: expected member name or ';' after declaration specifiers
281-  186 |     uint32_t nRandomXV2ActivationEpoch{2970}; END
282-      |                                               ~~~
283-  187 | };
--
310-[ 25%] Building CXX object src/CMakeFiles/bitcoin_node.dir/dbwrapper.cpp.o
311-In file included from /home/runner/work/scash/scash/src/consensus/tx_verify.cpp:7:
312-In file included from /home/runner/work/scash/scash/src/chain.h:11:
313:/home/runner/work/scash/scash/src/consensus/params.h:24:14: error: redefinition of 'nRuleChangeActivationThreshold'
314-   24 |     uint32_t nRuleChangeActivationThreshold{0};
315-      |              ^
316-/home/runner/work/scash/scash/src/consensus/params.h:19:14: note: previous definition is here
317-   19 |     uint32_t nRuleChangeActivationThreshold{0};
318-      |              ^
319:/home/runner/work/scash/scash/src/consensus/params.h:25:14: error: redefinition of 'nMinerConfirmationWindow'
320-   25 |     uint32_t nMinerConfirmationWindow{0};
321-      |              ^
322-/home/runner/work/scash/scash/src/consensus/params.h:20:14: note: previous definition is here
323-   20 |     uint32_t nMinerConfirmationWindow{0};
--- Tail of cmake_build.log ---
      |              ^
/home/runner/work/scash/scash/src/consensus/params.h:26:47: error: unknown type name 'END'
   26 |     uint32_t nRandomXV2ActivationEpoch{2970}; END
      |                                               ^
/home/runner/work/scash/scash/src/consensus/params.h:28:1: error: expected unqualified-id
   28 | namespace Consensus {
      | ^
/home/runner/work/scash/scash/src/consensus/params.h:184:14: error: duplicate member 'nRuleChangeActivationThreshold'
  184 |     uint32_t nRuleChangeActivationThreshold{0};
      |              ^
/home/runner/work/scash/scash/src/consensus/params.h:167:14: note: previous declaration is here
  167 |     uint32_t nRuleChangeActivationThreshold{0};
      |              ^
/home/runner/work/scash/scash/src/consensus/params.h:185:14: error: duplicate member 'nMinerConfirmationWindow'
  185 |     uint32_t nMinerConfirmationWindow{0};
      |              ^
/home/runner/work/scash/scash/src/consensus/params.h:168:14: note: previous declaration is here
  168 |     uint32_t nMinerConfirmationWindow{0};
      |              ^
/home/runner/work/scash/scash/src/consensus/params.h:186:14: error: duplicate member 'nRandomXV2ActivationEpoch'
  186 |     uint32_t nRandomXV2ActivationEpoch{2970}; END
      |              ^
/home/runner/work/scash/scash/src/consensus/params.h:169:14: note: previous declaration is here
  169 |     uint32_t nRandomXV2ActivationEpoch{2970};
      |              ^
/home/runner/work/scash/scash/src/consensus/params.h:186:47: error: unknown type name 'END'
  186 |     uint32_t nRandomXV2ActivationEpoch{2970}; END
      |                                               ^
/home/runner/work/scash/scash/src/consensus/params.h:187:1: error: expected member name or ';' after declaration specifiers
  186 |     uint32_t nRandomXV2ActivationEpoch{2970}; END
      |                                               ~~~
  187 | };
      | ^
10 errors generated.
gmake[2]: *** [src/CMakeFiles/bitcoin_node.dir/build.make:1367: src/CMakeFiles/bitcoin_node.dir/wallet/init.cpp.o] Error 1
gmake[2]: Target 'src/CMakeFiles/bitcoin_node.dir/build' not remade because of errors.
gmake[1]: *** [CMakeFiles/Makefile2:874: src/CMakeFiles/bitcoin_node.dir/all] Error 2
gmake[1]: Target 'all' not remade because of errors.
gmake: *** [Makefile:146: all] Error 2
gmake: Target 'default_target' not remade because of errors.
```
