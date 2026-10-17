# Scash v28.4 Build, Unit Test & Regtest Verification Report

## 1. Compiler & Configure Summary (exit code: 1)
```text
Ubuntu clang version 18.1.3 (1ubuntu1)
Target: x86_64-pc-linux-gnu
checking for rcc-qt5... no
checking for rcc5... no
checking for rcc... /usr/lib/x86_64-linux-gnu/qt5/bin/rcc
checking for lrelease-qt5... no
checking for lrelease5... no
checking for lrelease... /usr/lib/x86_64-linux-gnu/qt5/bin/lrelease
checking for lupdate-qt5... no
checking for lupdate5... no
checking for lupdate... /usr/lib/x86_64-linux-gnu/qt5/bin/lupdate
checking for lconvert-qt5... no
checking for lconvert5... no
checking for lconvert... /usr/lib/x86_64-linux-gnu/qt5/bin/lconvert
checking whether to build Scash GUI... yes (Qt5)
checking whether main function is needed for fuzz binary... checking whether the linker accepts ... no
yes
checking for Berkeley DB C++ headers... default
configure: WARNING: Found Berkeley DB other than 4.8
configure: WARNING: BDB (legacy) wallets opened by this build will not be portable!
checking for main in -ldb_cxx-4.8... no
checking for main in -ldb_cxx... yes
checking for sqlite3 >= 3.7.17... yes
checking whether to build wallet with support for sqlite... yes
checking whether Userspace, Statically Defined Tracing tracepoints are supported... no
checking for miniupnpc/miniupnpc.h... yes
checking for miniupnpc/upnpcommands.h... yes
checking for miniupnpc/upnperrors.h... yes
checking for upnpDiscover in -lminiupnpc... yes
checking whether miniUPnPc API version is supported... yes
checking RandomX dependency... checking for randomx.h... no
configure: error: RandomX dependency not built correctly
Setting up libxext-dev:amd64 (2:1.3.4-1build2) ...
Setting up libglx-dev:amd64 (1.7.0-1build1) ...
Setting up libgl-dev:amd64 (1.7.0-1build1) ...
Setting up libegl-dev:amd64 (1.7.0-1build1) ...
Setting up libglu1-mesa-dev:amd64 (9.0.2-1.1build1) ...
Setting up qtbase5-dev:amd64 (5.15.13+dfsg-1ubuntu1) ...
Setting up libqt5opengl5-dev:amd64 (5.15.13+dfsg-1ubuntu1) ...
Setting up qttools5-dev:amd64 (5.15.13-1) ...
Processing triggers for libc-bin (2.39-0ubuntu8.9) ...

Running kernel seems to be up-to-date.

Restarting services...
 systemctl restart packagekit.service php8.3-fpm.service

No containers need to be restarted.

No user sessions are running outdated binaries.

No VM guests are running outdated hypervisor (qemu) binaries on this host.
```

## 2. Build Outcome
COMPILER / BUILD LOG OUTPUT:
```text
--- Tail of build.log ---
```
