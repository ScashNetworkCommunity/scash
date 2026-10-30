Scash Core integration/staging tree
===================================

https://www.google.com/search?q=https://scash.network

For an immediately usable, binary version of the Scash Core software, see
https://github.com/ScashNetworkCommunity/scash/releases.

 For instructions on building Scash Core from source (using CMake on Bitcoin Core v29.3), see [doc/build-scash.md](https://www.google.com/search?q=https://github.com/ScashNetworkCommunity/scash/blob/scash_master/doc/build-scash.md).
For Scash release notes, see [doc/release-notes/scash/](https://www.google.com/search?q=https://github.com/ScashNetworkCommunity/scash/tree/scash_master/doc/release-notes/scash).

What is Scash Core?
-------------------

Scash (s/atoshi/cash) is a fork of Bitcoin Core which adds a new chain option (`-chain=scash`) using RandomX Proof-of-Work and per-block ASERT difficulty adjustment (`aserti3-2d`) to restore home computer CPU mining.

Scash Core connects to the Scash peer-to-peer network to download and fully
validate blocks and transactions. It also includes a wallet and graphical user
interface (`scash-qt`), which can be optionally built.

Further information about building and configuring Scash Core is available in [doc/build-scash.md](https://www.google.com/search?q=https://github.com/ScashNetworkCommunity/scash/blob/scash_master/doc/build-scash.md) and the [doc folder](https://www.google.com/search?q=https://github.com/ScashNetworkCommunity/scash/tree/scash_master/doc).

License
-------

Scash Core and Bitcoin Core are released under the terms of the MIT license. See [COPYING](https://www.google.com/search?q=https://github.com/ScashNetworkCommunity/scash/blob/scash_master/COPYING) for more
information or see https://opensource.org/license/MIT.

Development Process
-------------------

The `scash_master` branch is regularly built (see `doc/build-scash.md` and `doc/build-*.md` for instructions) and tested, but it is not guaranteed to be
completely stable. [Tags](https://www.google.com/search?q=https://github.com/ScashNetworkCommunity/scash/tags) are created
regularly from release branches to indicate new official, stable release versions of Scash Core.

The contribution workflow is described in [CONTRIBUTING.md](https://www.google.com/search?q=https://github.com/ScashNetworkCommunity/scash/blob/scash_master/CONTRIBUTING.md)
and useful hints for developers can be found in [doc/developer-notes.md](https://www.google.com/search?q=https://github.com/ScashNetworkCommunity/scash/blob/scash_master/doc/developer-notes.md).

Testing
-------

Testing and code review is the bottleneck for development; we get more pull
requests than we can review and test on short notice. Please be patient and help out by testing
other people's pull requests, and remember this is a security-critical project where any mistake might cost people
lots of money.

### Automated Testing

Developers are strongly encouraged to write [unit tests](https://www.google.com/search?q=https://github.com/ScashNetworkCommunity/scash/blob/scash_master/src/test/README.md) for new code, and to
submit new unit tests for old code. Unit tests can be compiled and run
(assuming they weren't disabled during the build generation) with: `ctest --test-dir build` (or `./build/bin/test_scash`). Further details on running
and extending unit tests can be found in [src/test/README.md](https://www.google.com/search?q=https://github.com/ScashNetworkCommunity/scash/blob/scash_master/src/test/README.md).

There are also [regression and integration tests](https://www.google.com/search?q=https://github.com/ScashNetworkCommunity/scash/tree/scash_master/test), written
in Python.
These tests can be run (if the [test dependencies](https://www.google.com/search?q=https://github.com/ScashNetworkCommunity/scash/tree/scash_master/test) are installed) with: `build/test/functional/test_runner.py`

The CI (Continuous Integration) systems make sure that every pull request is built for Windows, Linux, and macOS,
and that unit/sanity tests are run automatically.

### Manual Quality Assurance (QA) Testing

Changes should be tested by somebody other than the developer who wrote the
code. This is especially important for large or high-risk changes. It is useful
to add a test plan to the pull request description if testing the changes is
not straightforward.

Translations
------------

Changes to translations as well as new translations can be submitted to
[Bitcoin Core's Transifex page](https://www.google.com/search?q=https://www.transifex.com/bitcoin/bitcoin/).

Translations are periodically pulled from Transifex and merged into the git repository. See the
[translation process](https://www.google.com/search?q=https://github.com/ScashNetworkCommunity/scash/blob/scash_master/doc/translation_process.md) for details on how this works.

**Important**: We do not accept translation changes as GitHub pull requests because the next
pull from Transifex would automatically overwrite them again.
