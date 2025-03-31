######################################################################################
#                               Imports                                              #
######################################################################################
import glob
import os
import shutil
import subprocess

from invoke import task

######################################################################################
#                       Tool Configuration Variables                                 #
######################################################################################
C_COMPILER = "clang-19"
C_FORMATTER = "clang-format-19"
C_LINTTER = "clang-tidy-19"

######################################################################################
#                             Public API                                             #
######################################################################################
ROOT_PATH = os.path.dirname(os.path.abspath(__file__))
SRC_PATH = os.path.join(ROOT_PATH, "src")
BUILD_PATH = os.path.join(ROOT_PATH, "build")
TEST_PATH = os.path.join(ROOT_PATH, "test")


@task
def install(c):
    """
    Install dependencies if they are not already installed.

    Usage:
        inv install
    """
    dependencies = {
        "meson": "meson",
        "ruby": "ruby",
        C_COMPILER: C_COMPILER,
        C_FORMATTER: C_FORMATTER,
        C_LINTTER: C_LINTTER,
    }
    _pr_info("Installing dependencies...")

    for dep_cmd, dep_package in dependencies.items():
        if not _command_exists(dep_cmd):
            _pr_warn(f"{dep_cmd} not found. Installing {dep_package}...")
            _run_command(c, f"sudo apt-get install -y {dep_package}")
        else:
            _pr_info(f"{dep_package} already installed")

    _pr_info("Dependencies installed")


@task
def build(c):
    _pr_info("Building...")

    _run_command(c, f"mkdir -p {BUILD_PATH}")
    _run_command(c, f"CC={C_COMPILER} meson setup {BUILD_PATH}")
    _run_command(c, f"meson compile -C {BUILD_PATH}")

    _pr_info("Build done")


@task
def test(c):
    _pr_info("Testing...")

    _run_command(c, f"meson test -C {BUILD_PATH}")

    _pr_info("Testing done")


@task
def lint(c):
    patterns = [
        "src/**/*.c",
        "src/**/*.h",
    ]

    _pr_info("Linting...")

    for pattern in patterns:
        _pr_info(f"Linting files matching pattern '{pattern}'")

        for path in glob.glob(pattern, recursive=True):
            if os.path.isfile(path):
                _run_command(c, f"{C_LINTTER} -p {BUILD_PATH} {path}")
                _pr_info(f"{path} linted")

    _pr_info("Linting done")


@task
def format(c):
    patterns = [
        "src/**/*.c",
        "src/**/*.h",
    ]

    _pr_info("Formating...")

    for pattern in patterns:
        _pr_info(f"Formating files matching pattern '{pattern}'")

        for path in glob.glob(pattern, recursive=True):
            if os.path.isfile(path):
                _run_command(c, f"{C_FORMATTER} {path}")
                _pr_info(f"{path} formated")

    _pr_info("Formating done")


@task
def clean(c, extra=""):
    """
    Clean up build and temporary files recursively.

    This task removes specified patterns of files and directories,
    including build artifacts and temporary files.

    Args:
        extra (str, optional): Additional pattern to remove. Defaults to "".

    Usage:
        inv clean
        inv clean --bytecode
        inv clean --extra='**/*.log'
    """
    patterns = [
        "build/*",
        "**/*~",
        "**/#*",
        "*~",
        "#*",
    ]

    if extra:
        patterns.append(extra)

    for pattern in patterns:
        _pr_info(f"Removing files matching pattern '{pattern}'")

        for path in glob.glob(pattern, recursive=True):
            if os.path.isfile(path):
                os.remove(path)
                print(f"Removed file {path}")
            elif os.path.isdir(path):
                shutil.rmtree(path)
                print(f"Removed directory {path}")

    _pr_info("Clean up completed.")


######################################################################################
#                                  Private API                                       #
######################################################################################
def _get_file_extension(file_path):
    _, file_extension = os.path.splitext(file_path)
    return file_extension


def _command_exists(command):
    try:
        subprocess.run(
            [command, "--version"], stdout=subprocess.PIPE, stderr=subprocess.PIPE
        )
        return True
    except FileNotFoundError:
        return False
    except subprocess.CalledProcessError:
        return True
    except Exception:
        return False


def _run_command(c, command):
    _pr_debug(f"Executing '{command}'...")

    try:
        result = c.run(command, warn=True)

        if not result.ok:
            raise Exception("Result not ok")

    except Exception as exc:
        _pr_error(f"Command {command} failed: {exc}")
        exit(1)


def _cut_path_to_directory(full_path, target_directory):
    parts = full_path.split(os.sep)
    target_index = parts.index(target_directory)
    return os.sep.join(parts[: target_index + 1])


def _pr_info(message: str):
    print(f"\033[94m[INFO] {message}\033[0m")


def _pr_warn(message: str):
    print(f"\033[93m[WARN] {message}\033[0m")


def _pr_debug(message: str):
    print(f"\033[96m[DEBUG] {message}\033[0m")


def _pr_error(message: str):
    print(f"\033[91m[ERROR] {message}\033[0m")
