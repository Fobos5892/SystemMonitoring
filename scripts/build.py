#!/usr/bin/env python3
"""Configure and build SystemMonitoring with qmake/make.

Works in GitHub Actions (Linux) and locally (Windows / Linux / macOS).

Examples:
  python scripts/build.py tests --release --run-tests
  python scripts/build.py app --qt-dir C:/Qt/6.11.1/mingw_64 --debug
  python scripts/build.py all --release --jobs 8
"""

from __future__ import annotations

import argparse
import os
import platform
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

TARGETS: dict[str, Path] = {
    "tests": ROOT / "tests" / "tests.pro",
    "app": ROOT / "SystemMonitoring.pro",
    "all": ROOT / "Project.pro",
}

BINARY_NAMES: dict[str, str] = {
    "tests": "SystemMonitoringTests",
    "app": "SystemMonitoring",
    "all": "SystemMonitoringTests",
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Build SystemMonitoring with qmake/make.")
    parser.add_argument(
        "target",
        choices=sorted(TARGETS),
        help="tests: unit tests; app: GUI; all: app + tests (Project.pro)",
    )
    parser.add_argument(
        "--release",
        action="store_true",
        help="build in release mode (default)",
    )
    parser.add_argument(
        "--debug",
        action="store_true",
        help="build in debug mode",
    )
    parser.add_argument(
        "--qt-dir",
        type=Path,
        help="Qt kit root (directory that contains bin/qmake). Also reads QTDIR.",
    )
    parser.add_argument(
        "-j",
        "--jobs",
        type=int,
        default=0,
        help="parallel make jobs (default: CPU count)",
    )
    parser.add_argument(
        "--build-dir",
        type=Path,
        help="build directory (default: build/py/<target>-<config>)",
    )
    parser.add_argument(
        "--run-tests",
        action="store_true",
        help="run SystemMonitoringTests after a successful tests/all build",
    )
    parser.add_argument(
        "--clean",
        action="store_true",
        help="remove the build directory before configuring",
    )
    parser.add_argument(
        "--no-deploy",
        action="store_true",
        help="on Windows, skip windeployqt (by default DLLs are copied next to each exe)",
    )
    parser.add_argument(
        "--qmake-arg",
        action="append",
        default=[],
        metavar="ARG",
        help="extra argument passed to qmake (repeatable)",
    )
    return parser.parse_args()


def resolve_qt_bin(args: argparse.Namespace) -> Path:
    candidates: list[Path] = []
    if args.qt_dir:
        candidates.append(args.qt_dir)
    qt_dir = os.environ.get("QTDIR")
    if qt_dir:
        candidates.append(Path(qt_dir))

    for candidate in candidates:
        qmake = candidate / "bin" / qmake_name()
        if qmake.is_file():
            return candidate / "bin"

    qmake_in_path = shutil.which(qmake_name())
    if qmake_in_path:
        return Path(qmake_in_path).parent

    message = (
        "qmake not found. Add Qt bin to PATH, set QTDIR, or pass --qt-dir.\n"
        "Windows example:\n"
        "  python scripts/build.py tests --qt-dir C:/Qt/6.11.1/mingw_64 --release"
    )
    raise SystemExit(message)


def qmake_name() -> str:
    return "qmake.exe" if platform.system() == "Windows" else "qmake"


def detect_qt_version(qt_bin: Path) -> str:
    qmake = qt_bin / qmake_name()
    try:
        result = subprocess.run(
            [str(qmake), "-query", "QT_VERSION"],
            capture_output=True,
            text=True,
            check=True,
        )
        return result.stdout.strip()
    except (OSError, subprocess.CalledProcessError):
        return "unknown"


def make_program(env: dict[str, str]) -> str:
    path = env.get("PATH", "")
    if platform.system() == "Windows":
        for candidate in ("mingw32-make", "jom", "nmake"):
            resolved = shutil.which(candidate, path=path)
            if resolved:
                return resolved
        raise SystemExit("make tool not found (expected mingw32-make, jom, or nmake in PATH)")
    return shutil.which("make", path=path) or "make"


def run(command: list[str], *, env: dict[str, str], cwd: Path) -> None:
    print(f"+ {' '.join(command)}", flush=True)
    subprocess.run(command, cwd=cwd, env=env, check=True)


def binary_name(component: str) -> str:
    name = BINARY_NAMES[component]
    if platform.system() == "Windows":
        name += ".exe"
    return name


def binary_candidates(
    build_dir: Path,
    component: str,
    config: str,
    *,
    from_subdirs: bool = False,
) -> list[Path]:
    name = binary_name(component)
    bases = [build_dir]
    if from_subdirs and component == "tests":
        bases.insert(0, build_dir / "tests")

    candidates: list[Path] = []
    for base in bases:
        candidates.append(base / config / name)  # Windows MinGW, macOS
        candidates.append(base / name)           # Linux qmake

    unique: list[Path] = []
    for path in candidates:
        if path not in unique:
            unique.append(path)
    return unique


def resolve_binary(
    build_dir: Path,
    component: str,
    config: str,
    *,
    from_subdirs: bool = False,
) -> Path | None:
    for path in binary_candidates(build_dir, component, config, from_subdirs=from_subdirs):
        if path.is_file():
            return path.resolve()
    return None


def mingw_bin_from_qmake(qt_bin: Path) -> Path | None:
    qmake = qt_bin / qmake_name()
    try:
        result = subprocess.run(
            [str(qmake), "-query", "QMAKE_CXX"],
            capture_output=True,
            text=True,
            check=True,
        )
    except (OSError, subprocess.CalledProcessError):
        return None

    cxx = Path(result.stdout.strip().strip('"'))
    return cxx.parent if cxx.is_file() else None


def augment_path_with_mingw(qt_bin: Path, env: dict[str, str]) -> None:
    compiler_bin = mingw_bin_from_qmake(qt_bin)
    if compiler_bin is None:
        tools_dir = qt_bin.parent.parent.parent / "Tools"
        kit_name = qt_bin.parent.name
        preferred = "mingw810_64" if "mingw81" in kit_name else "mingw1310_64"
        for name in (preferred, "mingw1310_64", "mingw810_64", "mingw1120_64"):
            candidate = tools_dir / name / "bin"
            if candidate.is_dir():
                compiler_bin = candidate
                break

    if compiler_bin is not None:
        env["PATH"] = os.pathsep.join([str(compiler_bin), env["PATH"]])


def deploy_windows_binary(exe: Path, qt_bin: Path, env: dict[str, str]) -> None:
    windeployqt = qt_bin / "windeployqt.exe"
    if not windeployqt.is_file():
        print(f"skip deploy: {windeployqt} not found", flush=True)
        return

    command = [str(windeployqt), "--compiler-runtime", "--no-translations", str(exe)]
    run(command, env=env, cwd=exe.parent)


def deploy_windows_outputs(
    build_dir: Path,
    target: str,
    config: str,
    qt_bin: Path,
    env: dict[str, str],
) -> None:
    for label, path in collect_outputs(build_dir, target, config):
        resolved = path.resolve()
        if not resolved.is_file():
            continue
        print(f"Deploy ({label}): {resolved}", flush=True)
        deploy_windows_binary(resolved, qt_bin, env)


def build_config(args: argparse.Namespace) -> str:
    if args.debug and args.release:
        raise SystemExit("use either --debug or --release, not both")
    if args.debug:
        return "debug"
    return "release"


def should_run_tests(target: str, args: argparse.Namespace) -> bool:
    return args.run_tests and target in {"tests", "all"}


def collect_outputs(
    build_dir: Path,
    target: str,
    config: str,
) -> list[tuple[str, Path]]:
    from_subdirs = target == "all"
    outputs: list[tuple[str, Path]] = []

    if target in {"tests", "all"}:
        resolved = resolve_binary(build_dir, "tests", config, from_subdirs=from_subdirs)
        if resolved is not None:
            outputs.append(("tests", resolved))

    if target in {"app", "all"}:
        resolved = resolve_binary(build_dir, "app", config, from_subdirs=from_subdirs)
        if resolved is not None:
            outputs.append(("app", resolved))

    return outputs


def print_outputs(build_dir: Path, target: str, config: str) -> None:
    print(f"\nBuild OK: {build_dir}", flush=True)
    print("Output:", flush=True)

    outputs = collect_outputs(build_dir, target, config)
    if not outputs:
        expected = binary_candidates(
            build_dir,
            "tests" if target in {"tests", "all"} else "app",
            config,
            from_subdirs=target == "all",
        )
        raise SystemExit(f"build finished but no executable was found (expected one of: {expected})")

    for label, path in outputs:
        print(f"  {label}: {path}", flush=True)


def main() -> int:
    args = parse_args()
    config = build_config(args)
    pro_file = TARGETS[args.target]

    qt_bin = resolve_qt_bin(args)
    qt_version = detect_qt_version(qt_bin)

    build_dir = args.build_dir or (ROOT / "build" / "py" / f"{args.target}-qt{qt_version}-{config}")
    build_dir = build_dir.resolve()

    if args.clean and build_dir.exists():
        shutil.rmtree(build_dir)

    build_dir.mkdir(parents=True, exist_ok=True)

    env = os.environ.copy()
    env["PATH"] = os.pathsep.join([str(qt_bin), env.get("PATH", "")])
    augment_path_with_mingw(qt_bin, env)

    print(f"Qt {qt_version} ({qt_bin.parent.name})", flush=True)

    qmake = qt_bin / qmake_name()
    qmake_command = [str(qmake), str(pro_file), f"CONFIG+={config}", *args.qmake_arg]
    run(qmake_command, env=env, cwd=build_dir)

    jobs = args.jobs or os.cpu_count() or 4
    make = make_program(env)
    make_command = [make, f"-j{jobs}"]
    run(make_command, env=env, cwd=build_dir)

    from_subdirs = args.target == "all"

    if should_run_tests(args.target, args):
        test_binary = resolve_binary(build_dir, "tests", config, from_subdirs=from_subdirs)
        if test_binary is None:
            candidates = binary_candidates(build_dir, "tests", config, from_subdirs=from_subdirs)
            raise SystemExit(f"test binary not found (expected one of: {candidates})")
        run([str(test_binary), "-o", "-,txt"], env=env, cwd=build_dir)

    if platform.system() == "Windows" and not args.no_deploy:
        deploy_windows_outputs(build_dir, args.target, config, qt_bin, env)

    print_outputs(build_dir, args.target, config)
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except subprocess.CalledProcessError as error:
        print(f"\ncommand failed with exit code {error.returncode}", file=sys.stderr)
        raise SystemExit(error.returncode)
