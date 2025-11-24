"""
This script is for testing the SysY compiler.
It compiles SysY source files to LLVM IR, then to executables,
and checks the output against standard answers.
"""
import subprocess
import os
import sys
import argparse
from typing import Optional
from dataclasses import dataclass
from contextlib import ExitStack


@dataclass
class TestConfig:
    """Configuration for a single test run."""
    input_file: str
    output_file: str
    opt_level: int
    std_input: Optional[str]
    std_output: str
    act_output: str


SYSY = "bin/compiler"
TESTCASES_DIR = "testcase/functional"
TEST_OUTPUT_DIR = "test_output"
TOOLCHAINS_CONF = "toolchains.conf"

IR_TIMEOUT = "30"
ASM_TIMEOUT = "60"

MAX_FILENAME_LEN = 0

RISCV_GCC = "riscv64-unknown-elf-gcc"
RISCV_AR = "riscv64-unknown-elf-ar"
TEXT_ADDR = "0x90000000"


def load_toolchains_config():
    """Load toolchain configuration from toolchains.conf"""
    global RISCV_GCC, RISCV_AR, TEXT_ADDR

    if not os.path.exists(TOOLCHAINS_CONF):
        return

    try:
        with open(TOOLCHAINS_CONF, 'r', encoding='utf-8') as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith('#'):
                    continue

                if '=' in line:
                    key, value = line.split('=', 1)
                    key = key.strip()
                    value = value.strip()

                    if key == 'RISCV_GCC':
                        RISCV_GCC = value
                    elif key == 'RISCV_AR':
                        RISCV_AR = value
                    elif key == 'TEXT_ADDR':
                        TEXT_ADDR = value
    except IOError:
        pass


def print_test_status(test_name: str, status: str, final: bool = False):
    """
    Print test status with proper formatting and line clearing.

    Args:
        test_name: Name of the test file
        status: Current status message
        final: Whether this is the final status (will add newline)
    """
    global MAX_FILENAME_LEN
    padded_name = test_name.ljust(MAX_FILENAME_LEN)

    output = f"\r\033[K{padded_name}  {status}"

    if final:
        print(output, flush=True)
    else:
        print(output, end="", flush=True)


def check_file(file1, file2):
    """Compares two files and returns the result of diff."""
    try:
        result = subprocess.run(
            ["diff", "--strip-trailing-cr", "-b", "-q", file1, file2],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            check=False)
    except FileNotFoundError:
        print(f"\033[91mUnknown Error on \033[0m{file1}, \033[91mPlease check your output file\033[0m")
        return 1
    return result.returncode


def add_returncode(file, ret):
    """Appends the return code to the given file."""
    need_newline = False
    try:
        with open(file, "r", encoding="utf-8") as f:
            content = f.read()
            if len(content) > 0:
                if not content.endswith("\n"):
                    need_newline = True
    except IOError:
        print(f"\033[91mUnknown Error on \033[0m{file}, \033[91mPlease check your output file\033[0m")
        return False

    with open(file, "a+", encoding="utf-8") as f:
        if need_newline:
            f.write("\n")
        f.write(str(ret))
        f.write("\n")
    return False


def _compile_to_ir(src_file: str, target_file: str, opt_level: int, test_name: str):
    """Compiles the input SysY file to LLVM IR."""
    print_test_status(test_name, "Compiling sy to ir")
    res = subprocess.run([
        "timeout", IR_TIMEOUT,
        SYSY, src_file, "-llvm", "-o", target_file, f"-O{opt_level}"
    ], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)
    if res.returncode == 124:
        print_test_status(test_name, "\033[93mCompile Time Limit Exceed\033[0m", final=True)
        return False
    if res.returncode != 0:
        print_test_status(test_name, "\033[93mCompiler Error\033[0m", final=True)
        return False
    return True


def _check_ir_syntax(target_file: str, src_file: str, test_name: str):
    """Checks the syntax of the generated LLVM IR file."""
    print_test_status(test_name, "Checking IR syntax")
    res = subprocess.run(
        ["llvm-as", target_file, "-o", "/dev/null"],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)
    if res.returncode != 0:
        print_test_status(test_name, "\033[93mLLVM-IR Syntax Error\033[0m", final=True)
        return False
    return True


def _compile_ir_and_link(target_file: str, src_file: str, test_name: str):
    """Compiles IR to object file and links it into an executable."""
    print_test_status(test_name, "Compiling ir to object")
    res = subprocess.run(
        ["clang", target_file, "-c", "-o", "tmp.o", "-w"],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)
    if res.returncode != 0:
        print_test_status(test_name, "\033[93mOutput Error\033[0m", final=True)
        return False

    print_test_status(test_name, "Linking object to exec")
    res = subprocess.run([
        "clang", "tmp.o", "-o", "tmp.bin", "-static", "-L./lib", "-lsysy_x86"
    ], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)
    subprocess.run(["rm", "tmp.o"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)
    if res.returncode != 0:
        print_test_status(test_name, "\033[93mLink Error\033[0m", final=True)
        return False
    return True


def _run_ir_and_check(test_cfg: TestConfig, test_name: str):
    """Runs the executable generated from IR and checks its output."""
    print_test_status(test_name, "Executing")
    res = None
    try:
        with ExitStack() as stack:
            stdout_file = stack.enter_context(
                open(test_cfg.act_output, "w", encoding="utf-8"))
            stdin_file = None
            if test_cfg.std_input:
                stdin_file = stack.enter_context(
                    open(test_cfg.std_input, "r", encoding="utf-8"))

            res = subprocess.run(
                ["timeout", ASM_TIMEOUT, "./tmp.bin"],
                stdin=stdin_file,
                stdout=stdout_file,
                stderr=subprocess.DEVNULL,
                check=False
            )
    except IOError:
        print_test_status(test_name, "\033[91mIO Error\033[0m", final=True)
        return False
    finally:
        subprocess.run(["rm", "-f", "tmp.bin"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)

    if res is None:
        return False

    if res.returncode == 124:
        print_test_status(test_name, "\033[93mExecute Time Limit Exceed\033[0m", final=True)
        subprocess.run(["rm", test_cfg.act_output], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)
        return False
    if res.returncode == 139:
        subprocess.run(["rm", test_cfg.act_output], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)
        print_test_status(test_name, "\033[93mRuntime Error (Segmentation Fault)\033[0m", final=True)
        return False

    add_returncode(test_cfg.act_output, res.returncode)
    check = check_file(test_cfg.act_output, test_cfg.std_output)
    subprocess.run(["rm", test_cfg.act_output], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)
    if check != 0:
        print_test_status(test_name, "\033[91mWrong Answer\033[0m", final=True)
        return False
    return True


def _compile_to_asm(src_file: str, target_file: str, opt_level: int, test_name: str):
    """Compiles the input SysY file to RISC-V assembly."""
    print_test_status(test_name, "Compiling sy to asm")
    res = subprocess.run([
        "timeout", ASM_TIMEOUT,
        SYSY, src_file, "-S", "-o", target_file, f"-O{opt_level}"
    ], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)
    if res.returncode == 124:
        print_test_status(test_name, "\033[93mCompile Time Limit Exceed\033[0m", final=True)
        return False
    if res.returncode != 0:
        print_test_status(test_name, "\033[93mCompiler Error\033[0m", final=True)
        return False
    return True


def _compile_asm_and_link_riscv(target_file: str, src_file: str, test_name: str):
    """Compiles RISC-V assembly to object file and links it into an executable."""
    global RISCV_GCC, TEXT_ADDR
    
    print_test_status(test_name, "Compiling asm to object")
    # Compile .s to .o
    res = subprocess.run(
        [RISCV_GCC, target_file, "-c", "-o", "tmp.o", "-w"],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)
    if res.returncode != 0:
        print_test_status(test_name, "\033[93mAssembly Error\033[0m", final=True)
        return False

    print_test_status(test_name, "Linking object to exec")
    # Link .o to executable
    res = subprocess.run([
        RISCV_GCC, "tmp.o", "-o", "tmp.bin",
        "-L./lib", "-lsysy_riscv",
        "-static", "-mcmodel=medany",
        f"-Wl,--no-relax,-Ttext={TEXT_ADDR}"
    ], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)
    subprocess.run(["rm", "tmp.o"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)
    if res.returncode != 0:
        print_test_status(test_name, "\033[93mLink Error\033[0m", final=True)
        return False
    return True


def _run_riscv_and_check(test_cfg: TestConfig, test_name: str):
    """Runs the RISC-V executable with qemu and checks its output."""
    print_test_status(test_name, "Executing")
    res = None
    try:
        with ExitStack() as stack:
            stdout_file = stack.enter_context(
                open(test_cfg.act_output, "w", encoding="utf-8"))
            stdin_file = None
            if test_cfg.std_input:
                stdin_file = stack.enter_context(
                    open(test_cfg.std_input, "r", encoding="utf-8"))

            res = subprocess.run(
                ["timeout", ASM_TIMEOUT, "qemu-riscv64", "./tmp.bin"],
                stdin=stdin_file,
                stdout=stdout_file,
                stderr=subprocess.DEVNULL,
                check=False
            )
    except IOError:
        print_test_status(test_name, "\033[91mIO Error\033[0m", final=True)
        return False
    finally:
        subprocess.run(["rm", "-f", "tmp.bin"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)

    if res is None:
        return False

    if res.returncode == 124:
        print_test_status(test_name, "\033[93mExecute Time Limit Exceed\033[0m", final=True)
        subprocess.run(["rm", test_cfg.act_output], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)
        return False
    if res.returncode == 139:
        subprocess.run(["rm", test_cfg.act_output], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)
        print_test_status(test_name, "\033[93mRuntime Error (Segmentation Fault)\033[0m", final=True)
        return False

    add_returncode(test_cfg.act_output, res.returncode)
    check = check_file(test_cfg.act_output, test_cfg.std_output)
    subprocess.run(["rm", test_cfg.act_output], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)
    if check != 0:
        print_test_status(test_name, "\033[91mWrong Answer\033[0m", final=True)
        return False
    return True


def _compile_to_asm_arm(src_file: str, target_file: str, opt_level: int, test_name: str):
    """Compiles the input SysY file to AArch64 assembly."""
    print_test_status(test_name, "Compiling sy to asm")
    res = subprocess.run([
        "timeout", ASM_TIMEOUT,
        SYSY, src_file, "-S", "-o", target_file, f"-O{opt_level}",
        "-march", "aarch64"
    ], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)
    if res.returncode == 124:
        print_test_status(test_name, "\033[93mCompile Time Limit Exceed\033[0m", final=True)
        return False
    if res.returncode != 0:
        print_test_status(test_name, "\033[93mCompiler Error\033[0m", final=True)
        return False
    return True


def _compile_asm_and_link_arm(target_file: str, src_file: str, test_name: str):
    """Compiles AArch64 assembly to object file and links it into an executable."""
    print_test_status(test_name, "Compiling asm to object")
    # Compile .s to .o
    res = subprocess.run(
        ["aarch64-linux-gnu-gcc", target_file, "-c", "-o", "tmp.o", "-w"],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)
    if res.returncode != 0:
        print_test_status(test_name, "\033[93mAssembly Error\033[0m", final=True)
        return False

    print_test_status(test_name, "Linking object to exec")
    # Link .o to executable
    res = subprocess.run([
        "aarch64-linux-gnu-gcc", "tmp.o", "-o", "tmp.bin",
        "-L./lib", "-lsysy_aarch", "-static"
    ], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)
    subprocess.run(["rm", "tmp.o"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)
    if res.returncode != 0:
        print_test_status(test_name, "\033[93mLink Error\033[0m", final=True)
        return False
    return True


def _run_arm_and_check(test_cfg: TestConfig, test_name: str):
    """Runs the AArch64 executable with qemu and checks its output."""
    print_test_status(test_name, "Executing")
    res = None
    try:
        with ExitStack() as stack:
            stdout_file = stack.enter_context(
                open(test_cfg.act_output, "w", encoding="utf-8"))
            stdin_file = None
            if test_cfg.std_input:
                stdin_file = stack.enter_context(
                    open(test_cfg.std_input, "r", encoding="utf-8"))

            res = subprocess.run(
                ["timeout", ASM_TIMEOUT, "qemu-aarch64", "./tmp.bin"],
                stdin=stdin_file,
                stdout=stdout_file,
                stderr=subprocess.DEVNULL,
                check=False
            )
    except IOError:
        print_test_status(test_name, "\033[91mIO Error\033[0m", final=True)
        return False
    finally:
        subprocess.run(["rm", "-f", "tmp.bin"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)

    if res is None:
        return False

    if res.returncode == 124:
        print_test_status(test_name, "\033[93mExecute Time Limit Exceed\033[0m", final=True)
        subprocess.run(["rm", test_cfg.act_output], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)
        return False
    if res.returncode in [139, -11]: # segfault
        subprocess.run(["rm", test_cfg.act_output], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)
        print_test_status(test_name, "\033[93mRuntime Error (Segmentation Fault)\033[0m", final=True)
        return False

    add_returncode(test_cfg.act_output, res.returncode)
    check = check_file(test_cfg.act_output, test_cfg.std_output)
    subprocess.run(["rm", test_cfg.act_output], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)
    if check != 0:
        print_test_status(test_name, "\033[91mWrong Answer\033[0m", final=True)
        return False
    return True


def _execute_ir(test_cfg: TestConfig):
    """Full pipeline to compile, run, and check a SysY file via LLVM IR."""
    # Print test case name at the beginning
    test_name = os.path.basename(test_cfg.input_file)
    
    if not _compile_to_ir(test_cfg.input_file, test_cfg.output_file, test_cfg.opt_level, test_name):
        return False

    if not _check_ir_syntax(test_cfg.output_file, test_cfg.input_file, test_name):
        return False

    if not _compile_ir_and_link(test_cfg.output_file, test_cfg.input_file, test_name):
        return False

    if not _run_ir_and_check(test_cfg, test_name):
        return False

    print_test_status(test_name, "\033[92mAccepted\033[0m", final=True)
    return True


def _execute_riscv(test_cfg: TestConfig):
    """Full pipeline to compile, run, and check a SysY file via RISC-V assembly."""
    test_name = os.path.basename(test_cfg.input_file)
    
    if not _compile_to_asm(test_cfg.input_file, test_cfg.output_file, test_cfg.opt_level, test_name):
        return False

    if not _compile_asm_and_link_riscv(test_cfg.output_file, test_cfg.input_file, test_name):
        return False

    if not _run_riscv_and_check(test_cfg, test_name):
        return False

    print_test_status(test_name, "\033[92mAccepted\033[0m", final=True)
    return True


def _execute_arm(test_cfg: TestConfig):
    """Full pipeline to compile, run, and check a SysY file via AArch64 assembly."""
    test_name = os.path.basename(test_cfg.input_file)
    
    if not _compile_to_asm_arm(test_cfg.input_file, test_cfg.output_file, test_cfg.opt_level, test_name):
        return False

    if not _compile_asm_and_link_arm(test_cfg.output_file, test_cfg.input_file, test_name):
        return False

    if not _run_arm_and_check(test_cfg, test_name):
        return False

    print_test_status(test_name, "\033[92mAccepted\033[0m", final=True)
    return True


def main():
    """Main function to parse arguments and run tests."""
    load_toolchains_config()
    
    parser = argparse.ArgumentParser(
        description="SysY Compiler Testing Script")
    parser.add_argument("--group", default="Basic", choices=["Basic", "Advanced"],
                        help="Test case group to run.")
    parser.add_argument("--stage", default="llvm", choices=["llvm", "riscv", "arm"],
                        help="Testing stage.")
    parser.add_argument("--opt", default=0, type=int, choices=[0, 1, 2],
                        help="Optimization level.")
    args = parser.parse_args()

    test_dir = os.path.join(TESTCASES_DIR, args.group)
    if not os.path.isdir(test_dir):
        print(f"Test directory not found: {test_dir}")
        sys.exit(1)

    if not os.path.exists(TEST_OUTPUT_DIR):
        os.makedirs(TEST_OUTPUT_DIR)

    sy_files = [f for f in os.listdir(test_dir) if f.endswith(".sy")]
    passes_tests = 0

    global MAX_FILENAME_LEN
    MAX_FILENAME_LEN = max(len(f) for f in sy_files) if sy_files else 20

    exec_funcs = {
        "llvm": _execute_ir,
        "riscv": _execute_riscv,
        "arm": _execute_arm,
    }

    exec_func = exec_funcs[args.stage]

    output_ext = ".ll" if args.stage == "llvm" else ".s"

    for sy_file in sorted(sy_files, key=lambda x: int(x.split('_')[0])):
        base_name = os.path.splitext(sy_file)[0]
        src_file = os.path.join(test_dir, sy_file)
        std_output_file = os.path.join(test_dir, base_name + ".out")
        input_path = os.path.join(test_dir, base_name + ".in")

        test_cfg = TestConfig(
            input_file=src_file,
            output_file=os.path.join(
                TEST_OUTPUT_DIR, base_name + ("-O" + str(args.opt)) + output_ext),
            opt_level=args.opt,
            std_input=input_path if os.path.exists(input_path) else None,
            std_output=std_output_file,
            act_output=os.path.join(TEST_OUTPUT_DIR, base_name + ".act")
        )

        if exec_func(test_cfg):
            passes_tests += 1

    print("\n" + "="*30)
    print(f"\tGroup: {args.group}, Stage: {args.stage}, Opt Level: {args.opt}")
    print(f"\tPassed: {passes_tests} / {len(sy_files)}")
    if len(sy_files) > 0:
        pass_rate = (passes_tests / len(sy_files)) * 100
        print(f"\tPass Rate: {pass_rate:.2f}%")
    print("="*30)


if __name__ == "__main__":
    main()
