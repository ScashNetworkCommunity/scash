# Scash 3.0.0 (Mid-World) — Bitcoin Core v30.4 + RandomX v2 Upgrade Report

- **Branch**: `scash-30.4-Mid-World`
- **Target Core Release**: `v30.4` (Scash 3.0.0 "Mid-World")
- **RandomX Branch**: `ScashNetworkCommunity/RandomX@scash-v2` (`RANDOMX_PROGRAM_SIZE_V2=384`, `RANDOMX_ARGON_SALT="RandomX-Scash\x02"`)

## 1. Conflict Marker Check
All 31 merge conflict files resolved cleanly! Zero `<<<<<<<` markers remain in the repository.

## 2. CMake Configure Summary (exit code: 0)
```text
  DBus (GUI) .......................... ON
Tests:
  test_bitcoin ........................ ON
  test_bitcoin-qt ..................... ON
  bench_bitcoin ....................... OFF
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


-- Configuring done (20.7s)
-- Generating done (0.2s)
-- Build files have been written to: /home/runner/work/scash/scash/build
```

## 3. Build & Unit Test Outcome (exit code: 1)
```text
--- Tail of cmake_build.log ---
Unknown argument -k
Usage: cmake --build <dir>             [options] [-- [native-options]]
       cmake --build --preset <preset> [options] [-- [native-options]]
Options:
  <dir>          = Project binary directory to be built.
  --preset <preset>, --preset=<preset>
                 = Specify a build preset.
  --list-presets[=<type>]
                 = List available build presets.
  --parallel [<jobs>], -j [<jobs>]
                 = Build in parallel using the given number of jobs. 
                   If <jobs> is omitted the native build tool's 
                   default number is used.
                   The CMAKE_BUILD_PARALLEL_LEVEL environment variable
                   specifies a default parallel level when this option
                   is not given.
  -t <tgt>..., --target <tgt>...
                 = Build <tgt> instead of default targets.
  --config <cfg> = For multi-configuration tools, choose <cfg>.
  --clean-first  = Build target 'clean' first, then build.
                   (To clean only, use --target 'clean'.)
  --resolve-package-references={on|only|off}
                 = Restore/resolve package references during build.
  -v, --verbose  = Enable verbose output - if supported - including
                   the build commands to be executed. 
  --             = Pass remaining options to the native tool.
```
