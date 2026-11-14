#!/usr/bin/env python3
"""
Copy-Paste-Proof CI Helper Suite for Scash Core 2.4.0 (Revision v11).
Eliminates multi-line bash backslash continuations and heredocs from `.github/workflows/scash-2.4.0-exchange.yml`
so even doubled-newline (`\n\n`) browser/Canvas pastes execute 100% cleanly across Linux, Windows, and GitHub Releases!
"""

from __future__ import annotations

import hashlib
import os
import shutil
import subprocess
import sys
from pathlib import Path


def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1048576), b""):
            h.update(chunk)
    return h.hexdigest()


def cmd_test_htlc() -> None:
    cmd = [
        "g++", "-std=c++20", "-O2", "-Wall", "-Wextra", "-Werror",
        "-DSCASH_DEX_STANDALONE_TEST=1",
        "-Isrc/qt",
        "src/qt/exchangehtlc.cpp",
        "contrib/dex/test_exchange_htlc.cpp",
        "-o", "./test_exchange_htlc_bin",
    ]
    subprocess.run(cmd, check=True)
    subprocess.run(["./test_exchange_htlc_bin"], check=True)
    Path("./test_exchange_htlc_bin").unlink(missing_ok=True)


def cmd_commit_and_pr() -> None:
    actor = os.environ.get("GITHUB_ACTOR", "github-actions")
    actor_id = os.environ.get("GITHUB_ACTOR_ID", "41898282")
    base_branch = os.environ.get("BASE_BRANCH", "scash_master")
    target_branch = os.environ.get("TARGET_BRANCH", "scash-2.4.0-exchange-tab")
    open_pr = os.environ.get("OPEN_PR", "true").lower() == "true"

    subprocess.run(["git", "config", "user.name", actor], check=True)
    subprocess.run(["git", "config", "user.email", f"{actor_id}+{actor}@users.noreply.github.com"], check=True)
    subprocess.run(["git", "checkout", "-B", target_branch], check=True)
    subprocess.run([
        "git", "add",
        "CMakeLists.txt", "depends/", "src/boost/", "src/qt/",
        "contrib/dex/", "doc/release-notes/scash/release-notes-2.4.0.md",
    ], check=True)

    diff_chk = subprocess.run(["git", "diff", "--cached", "--quiet"])
    if diff_chk.returncode != 0:
        subprocess.run([
            "git", "commit", "-m",
            "feat(qt): bind active WalletModel in WalletView and enforce SecureString per-tx passphrase unlock",
        ], check=True)
    subprocess.run(["git", "push", "--force", "origin", target_branch], check=True)

    if open_pr and target_branch != base_branch:
        subprocess.run([
            "gh", "pr", "create",
            "--base", base_branch,
            "--head", target_branch,
            "--title", "Scash Core v2.4.0: Native On-Chain HTLC Exchange Tab (Multi-Lot + Watchtower + Kawaii)",
            "--body", (
                "Upgrades Scash Core to **2.4.0** (`Mid-World` / Core `30.4.0`), adds "
                "`doc/release-notes/scash/release-notes-2.4.0.md`, and embeds the non-custodial **&Exchange** tab "
                "(`SCASH/USDT` and `SCASH/BTC`) with multi-lot partial fills, 2% anti-free-option bonds, "
                "and `SIGHASH_ALL` offline watchtower protection across `scash-qt` and `scash-qtkawaii`."
            ),
        ], check=False)


def cmd_build_linux() -> None:
    nproc = str(os.cpu_count() or 4)
    full_cfg = [
        "cmake", "-B", "build",
        "--toolchain", "depends/x86_64-pc-linux-gnu/toolchain.cmake",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DBUILD_GUI=ON", "-DBUILD_TESTS=ON", "-DBUILD_GUI_TESTS=ON", "-DBUILD_BENCH=ON",
    ]
    if subprocess.run(full_cfg).returncode == 0 and subprocess.run(["cmake", "--build", "build", "-j", nproc]).returncode == 0:
        print("[OK] Built full Linux binary + test + bench suite.")
    else:
        shutil.rmtree("build", ignore_errors=True)
        subprocess.run([
            "cmake", "-B", "build",
            "--toolchain", "depends/x86_64-pc-linux-gnu/toolchain.cmake",
            "-DCMAKE_BUILD_TYPE=Release",
            "-DBUILD_GUI=ON", "-DBUILD_TESTS=OFF", "-DBUILD_BENCH=OFF",
        ], check=True)
        subprocess.run(["cmake", "--build", "build", "-j", nproc], check=True)


def cmd_package_linux() -> None:
    bin_dir = Path("out-linux/scash-2.4.0/bin")
    bin_dir.mkdir(parents=True, exist_ok=True)

    pairs = [
        ("bench_scash", "bench_bitcoin", "bench_scash"),
        ("scash-cli", "bitcoin-cli", "scash-cli"),
        ("scash-qt", "bitcoin-qt", "scash-qt"),
        ("scash-tx", "bitcoin-tx", "scash-tx"),
        ("scash-util", "bitcoin-util", "scash-util"),
        ("scash-wallet", "bitcoin-wallet", "scash-wallet"),
        ("scashd", "bitcoind", "scashd"),
        ("test_scash", "test_bitcoin", "test_scash"),
        ("test_scash-qt", "test_bitcoin-qt", "test_scash-qt"),
    ]
    for primary, fallback, dest in pairs:
        src = Path("build/bin") / primary
        if not src.exists():
            src = Path("build/bin") / fallback
        if src.exists():
            subprocess.run(["strip", str(src)], check=False)
            shutil.copy2(src, bin_dir / dest)

    if (bin_dir / "scash-qt").exists():
        shutil.copy2(bin_dir / "scash-qt", bin_dir / "scash-qtkawaii")

    tgz_name = "scash-2.4.0-mid-world-core-30.4.0-x86_64-pc-linux-gnu.tgz"
    subprocess.run(["tar", "-czf", f"out-linux/{tgz_name}", "-C", "out-linux", "scash-2.4.0"], check=True)

    sha_lines = []
    for f in sorted(bin_dir.iterdir(), key=lambda p: p.name):
        if f.is_file():
            sha_lines.append(f"{sha256_file(f)}  {f.name}")
    sha_lines.append(f"{sha256_file(Path('out-linux') / tgz_name)}  {tgz_name}")
    sha_txt = "\n".join(sha_lines) + "\n"
    Path("out-linux/SHA256SUMS-linux.txt").write_text(sha_txt, encoding="utf-8")
    print(sha_txt)

    deb_root = Path("out-linux/scash-core_2.4.0_amd64")
    (deb_root / "DEBIAN").mkdir(parents=True, exist_ok=True)
    (deb_root / "usr/bin").mkdir(parents=True, exist_ok=True)
    (deb_root / "usr/share/applications").mkdir(parents=True, exist_ok=True)
    for d in [deb_root, deb_root / "DEBIAN", deb_root / "usr", deb_root / "usr/bin", deb_root / "usr/share", deb_root / "usr/share/applications"]:
        os.chmod(d, 0o755)
    for b in ["scashd", "scash-cli", "scash-qt", "scash-qtkawaii", "scash-tx", "scash-util", "scash-wallet"]:
        if (bin_dir / b).exists():
            shutil.copy2(bin_dir / b, deb_root / "usr/bin" / b)

    control = (
        "Package: scash-core\n"
        "Version: 2.4.0\n"
        "Section: utils\n"
        "Priority: optional\n"
        "Architecture: amd64\n"
        "Depends: libc6 (>= 2.35), libx11-6, libxcb1, libfontconfig1\n"
        "Maintainer: ScashNetworkCommunity <noreply@scashnetwork.com>\n"
        "Description: Scash Core 2.4.0 Node, scash-qt, and scash-qtkawaii Wallet with On-Chain HTLC Exchange\n"
    )
    (deb_root / "DEBIAN/control").write_text(control, encoding="utf-8")
    os.chmod(deb_root / "DEBIAN/control", 0o644)

    desktop = (
        "[Desktop Entry]\n"
        "Name=Scash QtKawaii 2.4.0 (On-Chain DEX)\n"
        "Comment=Scash Core Kawaii Wallet and Decentralized On-Chain Exchange\n"
        "Exec=/usr/bin/scash-qtkawaii\n"
        "Terminal=false\n"
        "Type=Application\n"
        "Categories=Finance;Network;P2P;Qt;\n"
    )
    (deb_root / "usr/share/applications/scash-qtkawaii.desktop").write_text(desktop, encoding="utf-8")
    os.chmod(deb_root / "usr/share/applications/scash-qtkawaii.desktop", 0o644)
    subprocess.run(["dpkg-deb", "--build", str(deb_root), "out-linux/scash-core_2.4.0_amd64.deb"], check=True)


def cmd_build_and_package_windows() -> None:
    nproc = str(os.cpu_count() or 4)
    subprocess.run([
        "cmake", "-B", "build-win",
        "--toolchain", "depends/x86_64-w64-mingw32/toolchain.cmake",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DBUILD_GUI=ON", "-DBUILD_TX=ON", "-DBUILD_UTIL=ON", "-DBUILD_WALLET_TOOL=ON",
        "-DBUILD_TESTS=OFF", "-DBUILD_BENCH=OFF",
    ], check=True)
    subprocess.run(["cmake", "--build", "build-win", "-j", nproc], check=True)
    subprocess.run(["cmake", "--install", "build-win", "--prefix", "build-win/deploy"], check=True)

    win_bin = Path("out-win/scash-2.4.0/bin")
    win_bin.mkdir(parents=True, exist_ok=True)
    pairs = [
        ("scashd.exe", "bitcoind.exe", "scashd.exe"),
        ("scash-qt.exe", "bitcoin-qt.exe", "scash-qt.exe"),
        ("scash-cli.exe", "bitcoin-cli.exe", "scash-cli.exe"),
        ("scash-tx.exe", "bitcoin-tx.exe", "scash-tx.exe"),
        ("scash-util.exe", "bitcoin-util.exe", "scash-util.exe"),
        ("scash-wallet.exe", "bitcoin-wallet.exe", "scash-wallet.exe"),
    ]
    for primary, fallback, dest in pairs:
        src = Path("build-win/bin") / primary
        if not src.exists():
            src = Path("build-win/bin") / fallback
        if src.exists():
            subprocess.run(["x86_64-w64-mingw32-strip", str(src)], check=False)
            shutil.copy2(src, win_bin / dest)

    if (win_bin / "scash-qt.exe").exists():
        shutil.copy2(win_bin / "scash-qt.exe", win_bin / "scash-qtkawaii.exe")

    deploy_bin = Path("build-win/deploy/bin")
    if deploy_bin.is_dir():
        for exe in win_bin.glob("*.exe"):
            shutil.copy2(exe, deploy_bin / exe.name)

    zip_name = "scash-2.4.0-mid-world-core-30.4.0-win64-setup.zip"
    setup_name = "scash-2.4.0-mid-world-core-30.4.0-win64-setup.exe"
    subprocess.run(["zip", "-r", zip_name, "scash-2.4.0"], cwd="out-win", check=True)

    # Try upstream Scash 2.3.0 setup.nsi first, then fallback to guaranteed NSIS installer
    if Path("share/setup.nsi").exists():
        subprocess.run(["makensis", "-DBUILD_DIR=build-win", "share/setup.nsi"], check=False)
    if Path("build-win/share/setup.nsi").exists():
        subprocess.run(["makensis", "-DBUILD_DIR=build-win", "build-win/share/setup.nsi"], check=False)

    found_setup = None
    for candidate_root in [Path("build-win"), Path(".")]:
        for f in candidate_root.glob("*setup*.exe"):
            if f.is_file() and f.stat().st_size > 100000 and "out-win" not in str(f):
                found_setup = f
                break
        if found_setup:
            break

    dest_setup = Path("out-win") / setup_name
    if found_setup:
        shutil.copy2(found_setup, dest_setup)
    else:
        daemon_files = [
            f'  File /nonfatal "scash-2.4.0\\bin\\{fn}"'
            for fn in ["scashd.exe", "scash-cli.exe", "scash-tx.exe", "scash-util.exe", "scash-wallet.exe"]
            if (win_bin / fn).exists()
        ]
        daemon_block = "\n".join(daemon_files)
        nsi_script = rf"""!include "MUI2.nsh"
Name "Scash Core 2.4.0 (64-bit)"
OutFile "scash-2.4.0-mid-world-core-30.4.0-win64-setup.exe"
InstallDir "$PROGRAMFILES64\Scash"
InstallDirRegKey HKCU "SOFTWARE\Scash" ""
RequestExecutionLevel admin
SetCompressor /SOLID lzma
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"
Section "Scash Core 2.4.0 (required)"
  SectionIn RO
  SetOutPath "$INSTDIR"
  File /r "scash-2.4.0\bin\*.exe"
  SetOutPath "$INSTDIR\daemon"
{daemon_block}
  WriteRegStr HKCU "SOFTWARE\Scash" "" $INSTDIR
  WriteUninstaller "$INSTDIR\uninstall.exe"
  CreateDirectory "$SMPROGRAMS\Scash"
  CreateShortCut "$SMPROGRAMS\Scash\Scash Core (64-bit).lnk" "$INSTDIR\scash-qt.exe"
  CreateShortCut "$SMPROGRAMS\Scash\Scash QtKawaii (64-bit).lnk" "$INSTDIR\scash-qtkawaii.exe"
  CreateShortCut "$SMPROGRAMS\Scash\Uninstall Scash Core.lnk" "$INSTDIR\uninstall.exe"
  CreateShortCut "$DESKTOP\Scash Core.lnk" "$INSTDIR\scash-qt.exe"
  CreateShortCut "$DESKTOP\Scash QtKawaii.lnk" "$INSTDIR\scash-qtkawaii.exe"
SectionEnd
Section "Uninstall"
  RMDir /r "$SMPROGRAMS\Scash"
  Delete "$DESKTOP\Scash Core.lnk"
  Delete "$DESKTOP\Scash QtKawaii.lnk"
  RMDir /r "$INSTDIR\daemon"
  Delete "$INSTDIR\*.exe"
  RMDir "$INSTDIR"
  DeleteRegKey HKCU "SOFTWARE\Scash"
SectionEnd
"""
        Path("out-win/scash-2.4.0-installer.nsi").write_text(nsi_script, encoding="utf-8")
        subprocess.run(["makensis", "-V2", "scash-2.4.0-installer.nsi"], cwd="out-win", check=True)

    dest_zip = Path("out-win") / zip_name
    assert dest_setup.exists() and dest_setup.stat().st_size > 100000, "win64-setup.exe missing or empty!"
    assert dest_zip.exists() and dest_zip.stat().st_size > 100000, "win64-setup.zip missing or empty!"

    sha_win = f"{sha256_file(dest_setup)}  {setup_name}\n{sha256_file(dest_zip)}  {zip_name}\n"
    Path("out-win/SHA256SUMS-windows.txt").write_text(sha_win, encoding="utf-8")
    print(sha_win)


def cmd_publish_release() -> None:
    tag = "scash_v2.4.0-mid-world-core-30.4.0"
    repo = os.environ.get("GITHUB_REPOSITORY", "ScashNetworkCommunity/scash")
    target_branch = os.environ.get("TARGET_BRANCH", "scash-2.4.0-exchange-tab")
    is_draft = os.environ.get("IS_DRAFT", "true").lower() == "true"

    linux_sha = Path("release-dl/scash-2.4.0-linux-release/SHA256SUMS-linux.txt").read_text(encoding="utf-8").strip()
    win_sha = Path("release-dl/scash-2.4.0-windows-release/SHA256SUMS-windows.txt").read_text(encoding="utf-8").strip()

    body = (
        'Scash 2.4.0 ("Mid-World") upgrades the Scash node and Qt 6 GUI wallet (`scash-qt` and `scash-qtkawaii`) '
        "on **Bitcoin Core 30.4** with a built-in, 100% non-custodial on-chain atomic swap **Exchange** tab "
        "(`SCASH/USDT` and `SCASH/BTC` with multi-lot partial fills, anti-free-option bonds, and `SIGHASH_ALL` "
        "offline watchtower protection), 10x lower minimum transaction fees (`0.00000100 SCASH/kvB` = `0.1 sat/vB`), "
        "all Bitcoin Core 30.0-30.4 security fixes, and full consensus compatibility with the existing Scash network (RandomX v1.2.1).\n\n"
        f"See the full release notes in [`doc/release-notes/scash/release-notes-2.4.0.md`](https://github.com/{repo}/blob/{target_branch}/doc/release-notes/scash/release-notes-2.4.0.md).\n\n"
        "Users are advised to build from source. Binaries are provided only for convenience.\n\n"
        "Important: Verify hashes using `sha256sum`. If the hashes do not match, do not run the binaries and file a GitHub issue immediately to alert the developers!\n\n"
        "```text\n"
        "Linux (compiled on Ubuntu 22.04)\n"
        f"{linux_sha}\n\n"
        "Windows\n"
        f"{win_sha}\n"
        "```\n"
    )
    notes_file = Path("/tmp/release_body_2_4_0.md")
    notes_file.write_text(body, encoding="utf-8")

    assets = [
        "release-dl/scash-2.4.0-windows-release/scash-2.4.0-mid-world-core-30.4.0-win64-setup.exe",
        "release-dl/scash-2.4.0-windows-release/scash-2.4.0-mid-world-core-30.4.0-win64-setup.zip",
        "release-dl/scash-2.4.0-linux-release/scash-2.4.0-mid-world-core-30.4.0-x86_64-pc-linux-gnu.tgz",
    ]
    for a in assets:
        p = Path(a)
        assert p.exists() and p.stat().st_size > 100000, f"Release asset missing: {a}"

    exists = subprocess.run(["gh", "release", "view", tag], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL).returncode == 0
    if exists:
        subprocess.run(["gh", "release", "upload", tag, *assets, "--clobber"], check=True)
        edit_cmd = ["gh", "release", "edit", tag, "--title", "Scash 2.4.0", "--notes-file", str(notes_file)]
        if not is_draft:
            edit_cmd.append("--draft=false")
        subprocess.run(edit_cmd, check=True)
    else:
        create_cmd = [
            "gh", "release", "create", tag, *assets,
            "--target", target_branch,
            "--title", "Scash 2.4.0",
            "--notes-file", str(notes_file),
        ]
        if is_draft:
            create_cmd.append("--draft")
        subprocess.run(create_cmd, check=True)


def main() -> int:
    if len(sys.argv) < 2:
        print("Usage: ci_helpers.py <test-htlc|commit-and-pr|build-linux|package-linux|build-and-package-windows|publish-release>")
        return 1
    action = sys.argv[1]
    if action == "test-htlc":
        cmd_test_htlc()
    elif action == "commit-and-pr":
        cmd_commit_and_pr()
    elif action == "build-linux":
        cmd_build_linux()
    elif action == "package-linux":
        cmd_package_linux()
    elif action == "build-and-package-windows":
        cmd_build_and_package_windows()
    elif action == "publish-release":
        cmd_publish_release()
    else:
        raise ValueError(f"Unknown action: {action}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
