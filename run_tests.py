#!/usr/bin/env python3

'''
Test runner for `sysy-runtime-lib`.

Scans `test/*.c` files and runs corresponding executables from `build/test`.
'''

import re
import subprocess
import argparse
from pathlib import Path
import tomllib


def parse_args() -> argparse.Namespace:
  parser = argparse.ArgumentParser(
      description='Run tests for `sysy-runtime-lib`')
  parser.add_argument('--timeout', type=int, default=60,
                      help='Timeout for each test in seconds (default: 60)')
  parser.add_argument('--test-dir', type=str, default='test',
                      help='Directory containing test `.c` files (default: test)')
  parser.add_argument('--build-dir', type=str, default='build/test',
                      help='Directory containing built test executables (default: build/test)')
  return parser.parse_args()


class TestResult:
  PASSED = 'PASSED'
  EXECUTABLE_NOT_FOUND = 'EXECUTABLE NOT FOUND'
  TIMEOUT = 'TIMEOUT'
  EXIT_CODE_MISMATCH = 'EXIT CODE MISMATCH'
  STDOUT_MISMATCH = 'STDOUT MISMATCH'
  STDERR_MISMATCH = 'STDERR MISMATCH'
  TIMER_CHECK_FAILED = 'TIMER CHECK FAILED'
  RUNTIME_ERROR = 'RUNTIME ERROR'


def strip_timer_lines(stderr: str) -> str:
  '''
  Removes timer information lines from stderr.
  '''
  pattern = r'.*: \d+H-\d+M-\d+S-\d+us\n'
  return re.sub(pattern, '', stderr)


def check_timer_output(stderr: str) -> bool:
  '''
  Checks if stderr's last line contains timer info with at least one non-zero value.
  Pattern: `TOTAL: <digits>H-<digits>M-<digits>S-<digits>us`.
  '''
  lines = stderr.rstrip('\n').split('\n')
  if not lines:
    return False

  last_line = lines[-1]
  pattern = r'TOTAL: (\d+)H-(\d+)M-(\d+)S-(\d+)us'
  match = re.search(pattern, last_line)

  if not match:
    return False

  # Check if at least one number is non-zero
  values = [int(match.group(i)) for i in range(1, 5)]
  return any(v != 0 for v in values)


def load_toml_config(toml_path: Path) -> dict:
  '''
  Loads test configuration from TOML file.
  '''
  if not toml_path.exists():
    return {}

  with open(toml_path, 'rb') as f:
    return tomllib.load(f)


def load_file_content(file_path: Path) -> str | None:
  '''
  Loads content from a file if it exists.
  '''
  if not file_path.exists():
    return None

  with open(file_path, 'r') as f:
    return f.read()


def run_single_test(test_name: str, executable: Path, test_dir: Path,
                    timeout: int) -> tuple[str, str]:
  '''
  Runs a single test and return `(result, detail_message)`.
  '''
  base_name = test_dir / test_name

  # Load configuration files
  toml_config = load_toml_config(base_name.with_suffix('.toml'))
  stdin_content = load_file_content(base_name.with_suffix('.in'))
  expected_stdout = load_file_content(base_name.with_suffix('.out'))
  expected_stderr = load_file_content(base_name.with_suffix('.err'))

  expected_exit_code = toml_config.get('exit_code')
  check_timer = toml_config.get('check_timer', False)

  # Run the test
  try:
    result = subprocess.run(
        [str(executable)],
        input=stdin_content,
        capture_output=True,
        text=True,
        timeout=timeout
    )
  except subprocess.TimeoutExpired:
    return TestResult.TIMEOUT, f'exceeded {timeout}s'
  except Exception as e:
    return TestResult.RUNTIME_ERROR, str(e)

  actual_stdout = result.stdout
  actual_stderr = result.stderr
  actual_exit_code = result.returncode

  # Check exit code if specified
  # Handle signal differences: Linux uses 128+signal, macOS uses -signal
  if expected_exit_code is not None:
    exit_code_match = (actual_exit_code == expected_exit_code)
    # Check for signal equivalence (e.g., 134 == 128+6 on Linux, -6 on macOS)
    if not exit_code_match and expected_exit_code > 128:
      signal_num = expected_exit_code - 128
      exit_code_match = (actual_exit_code == -signal_num)
    if not exit_code_match and actual_exit_code > 128:
      signal_num = actual_exit_code - 128
      exit_code_match = (expected_exit_code == -signal_num)
    if not exit_code_match:
      return (TestResult.EXIT_CODE_MISMATCH,
              f'expected {expected_exit_code}, got {actual_exit_code}')

  # Check stdout if expected file exists
  if expected_stdout is not None:
    if actual_stdout != expected_stdout:
      return (TestResult.STDOUT_MISMATCH,
              f'stdout differs from expected')

  # Check stderr if expected file exists (strip timer lines first)
  if expected_stderr is not None:
    stripped_stderr = strip_timer_lines(actual_stderr)
    if stripped_stderr != expected_stderr:
      return (TestResult.STDERR_MISMATCH,
              f'stderr differs from expected')

  # Check timer output if required
  if check_timer:
    if not check_timer_output(actual_stderr):
      return (TestResult.TIMER_CHECK_FAILED,
              'timer output missing or all zeros')

  return TestResult.PASSED, ''


def discover_tests(test_dir: Path) -> list[str]:
  '''
  Discovers all `.c` test files in `test_dir`.
  Returns list of test names (relative paths without `.c` extension).
  '''
  tests = []
  for c_file in test_dir.rglob('*.c'):
    rel_path = c_file.relative_to(test_dir)
    test_name = str(rel_path.with_suffix(''))
    tests.append(test_name)
  return sorted(tests)


def main():
  args = parse_args()

  # Get project root (directory containing this script)
  project_root = Path(__file__).parent.resolve()
  test_dir = project_root / args.test_dir
  build_dir = project_root / args.build_dir

  if not test_dir.exists():
    print(f'Error: test directory {test_dir} does not exist')
    exit(1)

  # Discover tests
  tests = discover_tests(test_dir)

  if not tests:
    print('No tests found')
    exit(0)

  # Run tests and collect results
  results = {
      TestResult.PASSED: [],
      TestResult.EXECUTABLE_NOT_FOUND: [],
      TestResult.TIMEOUT: [],
      TestResult.EXIT_CODE_MISMATCH: [],
      TestResult.STDOUT_MISMATCH: [],
      TestResult.STDERR_MISMATCH: [],
      TestResult.TIMER_CHECK_FAILED: [],
      TestResult.RUNTIME_ERROR: [],
  }

  max_name_len = max(len(name) for name in tests)

  for test_name in tests:
    # Find executable
    executable = build_dir / test_name

    # Print test name
    print(f'{test_name:<{max_name_len}} ... ', end='', flush=True)

    if not executable.exists():
      result = TestResult.EXECUTABLE_NOT_FOUND
      detail = ''
    else:
      result, detail = run_single_test(test_name, executable, test_dir,
                                       args.timeout)

    results[result].append(test_name)

    # Print result
    if detail:
      print(f'{result} ({detail})')
    else:
      print(result)

  # Print summary
  print()
  print('Summary:')

  total = len(tests)
  passed = len(results[TestResult.PASSED])
  failed = total - passed

  print(f'  Total:  {total}')
  print(f'  Passed: {passed}')
  print(f'  Failed: {failed}')

  if failed > 0:
    print()
    print('Failed tests:')
    for result_type, test_list in results.items():
      if result_type != TestResult.PASSED and test_list:
        print(f'  {result_type}:')
        for test_name in test_list:
          print(f'    - {test_name}')

  # Exit with appropriate code
  exit(0 if failed == 0 else 1)


if __name__ == '__main__':
  main()
