#!/usr/bin/env bash

# Test runner for toydb

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# Global counters
TESTS_RUN=0
TESTS_PASSED=0
TESTS_FAILED=0

# Run a single test command and verify output
run_test() {
  local test_name="$1"
  local input="$2"
  local expected_output="$3"

  ((TESTS_RUN++))

  echo "Running test: $test_name"

  # Run the command and capture output
  local actual_output=$(echo -e "$input" | ./bin/db)

  # Compare with expected output (ignoring trailing whitespace)
  if diff -wB <(echo "$expected_output") <(echo "$actual_output") >/dev/null; then
    echo -e "${GREEN}PASS${NC}: $test_name"
    ((TESTS_PASSED++))
    return 0
  else
    echo -e "${RED}FAIL${NC}: $test_name"
    echo "Expected:"
    echo "$expected_output"
    echo "Actual:"
    echo "$actual_output"
    ((TESTS_FAILED++))
    return 1
  fi
}

# Test cases

test_insert_and_select() {
  local input="insert 1 user1 person1@example.com\nselect\n.exit"
  local expected_output="db > Executed.
db > (1 | user1 | person1@example.com)
Executed.
db > "
  run_test "Insert and Select" "$input" "$expected_output"
}

test_table_full() {
  local input=""
  for i in {1..1401}; do
    input+="insert $i user$i person$i@example.com\n"
  done
  input+=".exit"

  # Extract the FIRST "Table full" error
  local output=$(echo -e "$input" | ./bin/db | grep -m1 "Error: Table full.")

  if [[ "$output" == "db > Error: Table full." ]]; then
    echo -e "${GREEN}PASS${NC}: Table Full"
  else
    echo -e "${RED}FAIL${NC}: Table Full"
  fi
}

test_max_length_strings() {
  local long_username=$(printf '%*s' 32 | tr ' ' 'a')
  local long_email=$(printf '%*s' 255 | tr ' ' 'a')

  local input="insert 1 $long_username $long_email\nselect\n.exit"
  local expected_output="db > Executed.
db > (1 | $long_username | $long_email)
Executed.
db > "
  run_test "Max Length Strings" "$input" "$expected_output"
}

test_too_long_strings() {
  local long_username=$(printf '%*s' 33 | tr ' ' 'a')
  local long_email=$(printf '%*s' 256 | tr ' ' 'a')

  local input="insert 1 $long_username $long_email\nselect\n.exit"
  local expected_output="db > String is too long.
db > Executed.
db > "
  run_test "Too Long Strings" "$input" "$expected_output"
}

test_negative_id() {
  local input="insert -1 user1 person1@example.com\nselect\n.exit"
  local expected_output="db > ID must be positive.
db > Executed.
db > "
  run_test "Negative ID" "$input" "$expected_output"
}

# Run all tests
echo "Starting test suite..."

test_insert_and_select
test_table_full
test_max_length_strings
test_too_long_strings
test_negative_id

# Print summary
echo
echo "Test Summary:"
echo "Total:  $TESTS_RUN"
echo -e "${GREEN}Passed: $TESTS_PASSED${NC}"
echo -e "${RED}Failed: $TESTS_FAILED${NC}"

# Exit with error if any tests failed
if [ $TESTS_FAILED -gt 0 ]; then
  exit 1
fi
