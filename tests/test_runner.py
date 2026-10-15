import os
import subprocess
import sys

def main():
    compiler_path = os.path.normpath(
        "./build/vxc.exe" if os.name == "nt" and os.path.exists("./build/vxc.exe") else "./build/vxc"
    )

    if not os.path.exists(compiler_path):
        print(f"Error: Compiler binary not found at '{compiler_path}'. Run make first.")
        sys.exit(1)

    test_dir = "tests"
    passed = 0
    total = 0

    test_files = sorted(
        [f for f in os.listdir(test_dir) if f.endswith(".vxc")]
    )

    # Include examples
    all_tests = [("examples/hello.vxc", 0)]
    for f in test_files:
        path = os.path.join(test_dir, f)
        expected_code = 0 if f.startswith("valid_") else 1
        all_tests.append((path, expected_code))

    print(f"Running {len(all_tests)} tests on {compiler_path}...\n")

    for path, expected_code in all_tests:
        total += 1
        res = subprocess.run([compiler_path, "--check", path], capture_output=True, text=True)
        is_success = (res.returncode == 0) if expected_code == 0 else (res.returncode != 0)

        status_str = "PASS" if is_success else "FAIL"
        print(f"[{status_str}] {path} (expected exit: {expected_code}, got: {res.returncode})")
        if not is_success:
            print(f"       stdout: {res.stdout.strip()}")
            print(f"       stderr: {res.stderr.strip()}")
        else:
            passed += 1

    print(f"\nResult: {passed}/{total} tests passed.")
    if passed != total:
        sys.exit(1)

if __name__ == "__main__":
    main()
