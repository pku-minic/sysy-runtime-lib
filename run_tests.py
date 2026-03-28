#!/usr/bin/env python3

'''
Test runner for `sysy-runtime-lib`.

Scans `test/*.c` files and runs corresponding executables from `build/test`.
'''

import sys
import re
import subprocess
import argparse
from pathlib import Path
import tomllib
import shlex
import difflib


def parse_args() -> argparse.Namespace:
  parser = argparse.ArgumentParser(
      description='Run tests for `sysy-runtime-lib`')
  parser.add_argument('--timeout', type=int, default=60,
                      help='Timeout for each test in seconds (default: 60)')
  parser.add_argument('--test-dir', type=str, default='test',
                      help='Directory containing test `.c` files (default: test)')
  parser.add_argument('--build-dir', type=str, default='build/test',
                      help='Directory containing built test executables (default: build/test)')
  parser.add_argument('--runner', type=str, default='',
                      help='Command prefix to run test executables (e.g., qemu-riscv32)')
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


def make_unified_diff(expected: str, actual: str, expected_name: str,
                      actual_name: str, indent: str) -> str:
  '''
  Builds a unified diff string between expected and actual text.
  '''
  diff_lines = difflib.unified_diff(
      expected.splitlines(),
      actual.splitlines(),
      fromfile=expected_name,
      tofile=actual_name,
      lineterm=''
  )
  if diff_lines:
    # Add additional indentation for better readability in test output
    diff_lines = [f'{indent}{line}' for line in diff_lines]
  diff_text = '\n'.join(diff_lines)
  return diff_text if diff_text else '(no textual diff)'


def strip_timer_lines(stderr: str) -> str:
  '''
  Removes timer information lines from stderr.
  '''
  pattern = r'.*: \d+H-\d+M-\d+S-\d+us\n'
  return re.sub(pattern, '', stderr)


def strip_qemu_uncaught_signal_lines(stderr: str) -> str:
  '''
  Removes qemu-user noise lines that start with
  `qemu: uncaught target signal`.
  '''
  lines = stderr.splitlines(keepends=True)
  filtered = [line for line in lines
              if not line.startswith('qemu: uncaught target signal')]
  return ''.join(filtered)


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
                    timeout: int, runner: str) -> tuple[str, str]:
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

  # Build command with optional runner prefix
  if runner:
    cmd = shlex.split(runner) + [str(executable)]
  else:
    cmd = [str(executable)]

  # Run the test
  try:
    result = subprocess.run(
        cmd,
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
  actual_stderr = strip_qemu_uncaught_signal_lines(result.stderr)
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
      diff_text = make_unified_diff(
          expected_stdout,
          actual_stdout,
          'expected/stdout',
          'actual/stdout',
          '|   ',
      )
      return (TestResult.STDOUT_MISMATCH,
              f'| stdout differs from expected\n{diff_text}')

  # Check stderr if expected file exists (strip timer lines first)
  if expected_stderr is not None:
    stripped_stderr = strip_timer_lines(actual_stderr)
    if stripped_stderr != expected_stderr:
      diff_text = make_unified_diff(
          expected_stderr,
          stripped_stderr,
          'expected/stderr',
          'actual/stderr (timer-stripped)',
          '|   ',
      )
      return (TestResult.STDERR_MISMATCH,
              f'| stderr differs from expected\n{diff_text}')

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
    sys.exit(1)

  # Discover tests
  tests = discover_tests(test_dir)

  if not tests:
    print('No tests found')
    sys.exit(0)

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
                                       args.timeout, args.runner)

    results[result].append(test_name)

    # Print result
    if detail:
      if '\n' in detail:
        print(result)
        print(detail)
      else:
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
  sys.exit(0 if failed == 0 else 1)


if __name__ == '__main__':
  main()
