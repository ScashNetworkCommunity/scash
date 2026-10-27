# SCASH BUILD NOTES

Scash (s/atoshi/cash) is a fork of Bitcoin Core which adds a new chain option to restore home computer mining.

Technical details are documented in the [Scash Protocol spec](https://github.com/ScashNetworkCommunity/sips/blob/main/scash-protocol-spec.md).

Building Scash follows the same instructions as building Bitcoin Core. Starting in Scash v2.2.0 ("Fillory"), Scash is based on Bitcoin Core v29.3 and uses the **CMake** build system (`cmake -B build`) instead of Autotools (`./autogen.sh` / `./configure`).

The Linux version of the node `scashd` and GUI app `scash-qt` are both supported, with Windows 64-bit binaries also available (cross-compiled on Linux). macOS is not yet officially supported. Windows users can also build from source by following the Linux instructions inside Ubuntu on [Windows Subsystem for Linux (WSL)](https://learn.microsoft.com/en-us/windows/wsl/about).

For more specific upstream instructions on building, see [build-unix.md](https://www.google.com/search?q=build-unix.md) and [build-windows.md](https://www.google.com/search?q=build-windows.md) in this directory.

Also see the latest [Scash release notes](https://www.google.com/search?q=release-notes/scash/).

## Getting started

Update your system and install the required build tools and dependencies (Ubuntu 22.04 / 24.04 or Debian 12):

```bash
sudo apt update
sudo apt upgrade
sudo apt install build-essential libtool autotools-dev automake pkg-config bsdmainutils python3 curl git cmake ninja-build bison flex gperf xz-utils libxcb-xinerama0-dev
```

## WSL for Windows

Ignore this step if building on native Linux. The following only applies when building in WSL for Windows (so Windows `PATH` entries containing spaces do not interfere with the `depends` build).

Open the WSL configuration file:
```bash
sudo nano /etc/wsl.conf
```

Add the following lines to the file:
```ini
[interop]
appendWindowsPath=false
```

Exit WSL and then restart WSL (`wsl --shutdown` in PowerShell).

## Downloading the code

Download the latest version of Scash and checkout the branch or release tag you intend to build:

```bash
git clone https://github.com/ScashNetworkCommunity/scash.git
cd scash
git checkout scash_master
```

## Building for Linux

Scash requires building its bundled dependencies (including RandomX) with the `depends` system first.

### Node software without the GUI

To build only the headless node and CLI utilities (`scashd`, `scash-cli`, `scash-tx`, `scash-wallet`, `scash-util`) without the Qt GUI app:

```bash
make -C depends NO_QT=1 NO_ZMQ=1 NO_USDT=1 NO_IPC=1 -j$(nproc)
cmake -B build --toolchain depends/x86_64-pc-linux-gnu/toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_GUI=OFF
cmake --build build -j$(nproc)
```

### Node software with the GUI

To build both the node software `scashd` and the Qt desktop GUI app `scash-qt`:

```bash
make -C depends NO_ZMQ=1 NO_USDT=1 NO_IPC=1 -j$(nproc)
cmake -B build --toolchain depends/x86_64-pc-linux-gnu/toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_GUI=ON
cmake --build build -j$(nproc)
```

### Running unit tests

To run the Scash unit test suite after building:

```bash
./build/bin/test_scash
```

### Executables

The compiled Linux executables are placed in **`build/bin/`**:
* `build/bin/scashd` — Scash headless daemon
* `build/bin/scash-qt` — Scash desktop GUI wallet (when built with `-DBUILD_GUI=ON`)
* `build/bin/scash-cli` — RPC command-line client
* `build/bin/scash-tx` — Raw transaction utility
* `build/bin/scash-wallet` — Offline wallet tool
* `build/bin/scash-util` — Utility tool
* `build/bin/test_scash` — Unit test runner

You can copy the binaries in `build/bin/` to a folder on your `PATH` (such as `/usr/local/bin/` or `$HOME/.local/bin/`), or install them system-wide with:

```bash
sudo cmake --install build
```

## Building for Windows (by cross-compiling on Linux)

To cross-compile 64-bit Windows executables (`.exe`) on Ubuntu/Debian Linux:

```bash
sudo apt install g++-mingw-w64-x86-64-posix nsis zip
sudo update-alternatives --set x86_64-w64-mingw32-gcc /usr/bin/x86_64-w64-mingw32-gcc-posix
sudo update-alternatives --set x86_64-w64-mingw32-g++ /usr/bin/x86_64-w64-mingw32-g++-posix

make -C depends HOST=x86_64-w64-mingw32 NO_ZMQ=1 NO_USDT=1 NO_IPC=1 -j$(nproc)
cmake -B build-win --toolchain depends/x86_64-w64-mingw32/toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_GUI=ON
cmake --build build-win -j$(nproc)
```

The compiled Windows executables (`scashd.exe`, `scash-qt.exe`, `scash-cli.exe`, `scash-tx.exe`, `scash-wallet.exe`, `scash-util.exe`, `test_scash.exe`) will be found in **`build-win/bin/`**.

To strip debug symbols from the Windows executables (recommended to reduce file size):

```bash
x86_64-w64-mingw32-strip build-win/bin/*.exe
```

## Config file

The Scash configuration file (`scash.conf`) uses the same options as `bitcoin.conf`.

By default, Scash looks for `scash.conf` in the default data directory:
* **Linux**: `$HOME/.scash/scash.conf`
* **Windows (fresh install on Scash 2.1.0+)**: `%LOCALAPPDATA%\Scash\scash.conf` (`C:\Users\USERNAME\AppData\Local\Scash\scash.conf`)
* **Windows (upgrading with an existing Roaming folder)**: `%APPDATA%\Scash\scash.conf` (`C:\Users\USERNAME\AppData\Roaming\Scash\scash.conf`)

The following is a sample `scash.conf`:
```ini
rpcuser=user
rpcpassword=password
chain=scash
daemon=1
debug=1
txindex=1

[scash]
adddnsseed=seed.scash.one

[scashtestnet]
adddnsseed=testnet.seed.scash.one
```

### Connecting to the network

Mainnet DNS seed `seed.scash.one`, testnet DNS seed `testnet.seed.scash.one`, and fallback fixed seed nodes are built into Scash Core by default, so fresh installations connect automatically without requiring a custom `scash.conf`. Additional peer addresses can also be added using `-addnode` or `-seednode`.

If you intend to use the same configuration file with multiple networks, the config sections are named as follows:
```ini
[btc]
[btctestnet3]
[btctestnet4]
[btcsignet]
[btcregtest]
[scash]
[scashregtest]
[scashtestnet]
```

## Running a node

To run the Scash node:
```bash
scashd
```

To send commands to the Scash node:
```bash
scash-cli [COMMAND] [PARAMETERS]
```

To run the desktop GUI app:
```bash
scash-qt
```

On WSL for Windows, launching `scash-qt` may require installing the following dependencies. Also see [WSL GUI apps](https://learn.microsoft.com/en-us/windows/wsl/tutorials/gui-apps).
```bash
sudo apt install libxcb-* libxkbcommon-x11-0
```

Also note that in WSL for Windows, by default only half of system memory is available to WSL. You can [configure the memory limit](https://learn.microsoft.com/en-us/windows/wsl/wsl-config#main-wsl-settings) by creating a `.wslconfig` file in your Windows user folder:
```ini
[wsl2]
memory=16GB
```

## Connecting to different chains

When running executables with the name `scash...` (`scashd`, `scash-qt`, `scash-cli`), if no chain is configured, the default chain is Scash mainnet (`scash`).

Option `-chain=` accepts the following values:
* **Scash networks**: `scash`, `scashtestnet`, `scashregtest`
* **Bitcoin networks**: `main`, `test`, `testnet4`, `signet`, `regtest`

## Mining Scash

There are a few ways to mine Scash.

### Testnet and Regtest chain

Mining can take place directly inside the Scash node using the `generatetoaddress` RPC (single-threaded). For example:
```bash
scash-cli createwallet myfirstwallet
scash-cli getnewaddress
scash-cli generatetoaddress 1 newminingaddress 10000
```

To speed up internal RPC mining in the Scash node at the expense of using more memory (at least 2 GB more), enable `randomxfastmode` in `scash.conf`:

```ini
randomxfastmode=1
```

### Main network and Testnet chain

Mining on mainnet takes place using [cpuminer-scash](https://github.com/ScashNetworkCommunity/cpuminer-scash), dedicated CPU mining software that connects to the Scash node and retrieves mining jobs via the `getblocktemplate` RPC. The `randomxfastmode` option is not required on the Scash node when using `cpuminer-scash`, since mining occurs inside `cpuminer-scash` (which runs in fast mode).

### Mining Pools

Third-party software exists for mining at pools.

## Getting Help

Please file a [GitHub issue](https://www.google.com/search?q=https://github.com/ScashNetworkCommunity/scash/issues) if build problems are not resolved after reviewing the available Scash and Bitcoin Core documentation.
