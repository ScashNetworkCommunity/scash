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


-- Configuring done (20.3s)
-- Generating done (0.2s)
-- Build files have been written to: /home/runner/work/scash/scash/build
```

## 2. Build & Unit Test Outcome (exit code: 2)
COMPILER / LINKER ERRORS:
```text
320-In file included from /home/runner/work/scash/scash/src/wallet/db.cpp:6:
321-In file included from /home/runner/work/scash/scash/src/chainparams.h:9:
322-In file included from /home/runner/work/scash/scash/src/kernel/chainparams.h:10:
323:/home/runner/work/scash/scash/src/consensus/params.h:24:14: error: redefinition of 'nRuleChangeActivationThreshold'
324-   24 |     uint32_t nRuleChangeActivationThreshold{0};
325-      |              ^
326-/home/runner/work/scash/scash/src/consensus/params.h:19:14: note: previous definition is here
327-   19 |     uint32_t nRuleChangeActivationThreshold{0};
328-      |              ^
329:/home/runner/work/scash/scash/src/consensus/params.h:25:14: error: redefinition of 'nMinerConfirmationWindow'
330-   25 |     uint32_t nMinerConfirmationWindow{0};
331-      |              ^
332-/home/runner/work/scash/scash/src/consensus/params.h:20:14: note: previous definition is here
333-   20 |     uint32_t nMinerConfirmationWindow{0};
334-      |              ^
335:/home/runner/work/scash/scash/src/consensus/params.h:26:14: error: redefinition of 'nRandomXV2ActivationEpoch'
336-   26 |     uint32_t nRandomXV2ActivationEpoch{2970}; END
337-      |              ^
338-/home/runner/work/scash/scash/src/consensus/params.h:21:14: note: previous definition is here
339-   21 |     uint32_t nRandomXV2ActivationEpoch{2970};
340-      |              ^
341:/home/runner/work/scash/scash/src/consensus/params.h:26:47: error: unknown type name 'END'
342-   26 |     uint32_t nRandomXV2ActivationEpoch{2970}; END
343-      |                                               ^
344:/home/runner/work/scash/scash/src/consensus/params.h:28:1: error: expected unqualified-id
345-   28 | namespace Consensus {
346-      | ^
347:/home/runner/work/scash/scash/src/consensus/params.h:184:14: error: duplicate member 'nRuleChangeActivationThreshold'
348-  184 |     uint32_t nRuleChangeActivationThreshold{0};
349-      |              ^
350-/home/runner/work/scash/scash/src/consensus/params.h:167:14: note: previous declaration is here
351-  167 |     uint32_t nRuleChangeActivationThreshold{0};
352-      |              ^
353:/home/runner/work/scash/scash/src/consensus/params.h:185:14: error: duplicate member 'nMinerConfirmationWindow'
354-  185 |     uint32_t nMinerConfirmationWindow{0};
355-      |              ^
356-/home/runner/work/scash/scash/src/consensus/params.h:168:14: note: previous declaration is here
357-  168 |     uint32_t nMinerConfirmationWindow{0};
358-      |              ^
359:/home/runner/work/scash/scash/src/consensus/params.h:186:14: error: duplicate member 'nRandomXV2ActivationEpoch'
360-  186 |     uint32_t nRandomXV2ActivationEpoch{2970}; END
361-      |              ^
362-/home/runner/work/scash/scash/src/consensus/params.h:169:14: note: previous declaration is here
363-  169 |     uint32_t nRandomXV2ActivationEpoch{2970};
364-      |              ^
365:/home/runner/work/scash/scash/src/consensus/params.h:186:47: error: unknown type name 'END'
366-  186 |     uint32_t nRandomXV2ActivationEpoch{2970}; END
367-      |                                               ^
368:/home/runner/work/scash/scash/src/consensus/params.h:187:1: error: expected member name or ';' after declaration specifiers
369-  186 |     uint32_t nRandomXV2ActivationEpoch{2970}; END
370-      |                                               ~~~
371-  187 | };
--
376-[ 39%] Building CXX object src/CMakeFiles/bitcoin_common.dir/chainparams.cpp.o
377-In file included from /home/runner/work/scash/scash/src/chain.cpp:6:
378-In file included from /home/runner/work/scash/scash/src/chain.h:11:
379:/home/runner/work/scash/scash/src/consensus/params.h:24:14: error: redefinition of 'nRuleChangeActivationThreshold'
380-   24 |     uint32_t nRuleChangeActivationThreshold{0};
381-      |              ^
382-/home/runner/work/scash/scash/src/consensus/params.h:19:14: note: previous definition is here
383-   19 |     uint32_t nRuleChangeActivationThreshold{0};
384-      |              ^
385:/home/runner/work/scash/scash/src/consensus/params.h:25:14: error: redefinition of 'nMinerConfirmationWindow'
386-   25 |     uint32_t nMinerConfirmationWindow{0};
387-      |              ^
388-/home/runner/work/scash/scash/src/consensus/params.h:20:14: note: previous definition is here
389-   20 |     uint32_t nMinerConfirmationWindow{0};
390-      |              ^
391:/home/runner/work/scash/scash/src/consensus/params.h:26:14: error: redefinition of 'nRandomXV2ActivationEpoch'
392-   26 |     uint32_t nRandomXV2ActivationEpoch{2970}; END
393-      |              ^
394-/home/runner/work/scash/scash/src/consensus/params.h:21:14: note: previous definition is here
395-   21 |     uint32_t nRandomXV2ActivationEpoch{2970};
396-      |              ^
397:/home/runner/work/scash/scash/src/consensus/params.h:26:47: error: unknown type name 'END'
398-   26 |     uint32_t nRandomXV2ActivationEpoch{2970}; END
399-      |                                               ^
400:/home/runner/work/scash/scash/src/consensus/params.h:28:1: error: expected unqualified-id
401-   28 | namespace Consensus {
402-      | ^
403:/home/runner/work/scash/scash/src/consensus/params.h:184:14: error: duplicate member 'nRuleChangeActivationThreshold'
404-  184 |     uint32_t nRuleChangeActivationThreshold{0};
405-      |              ^
406-/home/runner/work/scash/scash/src/consensus/params.h:167:14: note: previous declaration is here
407-  167 |     uint32_t nRuleChangeActivationThreshold{0};
408-      |              ^
409:/home/runner/work/scash/scash/src/consensus/params.h:185:14: error: duplicate member 'nMinerConfirmationWindow'
410-  185 |     uint32_t nMinerConfirmationWindow{0};
411-      |              ^
412-/home/runner/work/scash/scash/src/consensus/params.h:168:14: note: previous declaration is here
413-  168 |     uint32_t nMinerConfirmationWindow{0};
414-      |              ^
415:/home/runner/work/scash/scash/src/consensus/params.h:186:14: error: duplicate member 'nRandomXV2ActivationEpoch'
416-  186 |     uint32_t nRandomXV2ActivationEpoch{2970}; END
417-      |              ^
418-/home/runner/work/scash/scash/src/consensus/params.h:169:14: note: previous declaration is here
419-  169 |     uint32_t nRandomXV2ActivationEpoch{2970};
420-      |              ^
421:/home/runner/work/scash/scash/src/consensus/params.h:186:47: error: unknown type name 'END'
422-  186 |     uint32_t nRandomXV2ActivationEpoch{2970}; END
423-      |                                               ^
424:/home/runner/work/scash/scash/src/consensus/params.h:187:1: error: expected member name or ';' after declaration specifiers
425-  186 |     uint32_t nRandomXV2ActivationEpoch{2970}; END
426-      |                                               ~~~
427-  187 | };
--
477-In file included from /home/runner/work/scash/scash/src/test/util/blockfilter.cpp:7:
478-In file included from /home/runner/work/scash/scash/src/chainparams.h:9:
479-In file included from /home/runner/work/scash/scash/src/kernel/chainparams.h:10:
480:/home/runner/work/scash/scash/src/consensus/params.h:24:14: error: redefinition of 'nRuleChangeActivationThreshold'
481-   24 |     uint32_t nRuleChangeActivationThreshold{0};
482-      |              ^
483-/home/runner/work/scash/scash/src/consensus/params.h:19:14: note: previous definition is here
484-   19 |     uint32_t nRuleChangeActivationThreshold{0};
485-      |              ^
486:/home/runner/work/scash/scash/src/consensus/params.h:25:14: error: redefinition of 'nMinerConfirmationWindow'
487-   25 |     uint32_t nMinerConfirmationWindow{0};
488-      |              ^
489-/home/runner/work/scash/scash/src/consensus/params.h:20:14: note: previous definition is here
490-   20 |     uint32_t nMinerConfirmationWindow{0};
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
