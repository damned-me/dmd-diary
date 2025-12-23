#!/bin/bash
#
# Test runner for dry diary CLI
# Run with: ./tests/test_runner.sh
#

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Counters
TESTS_RUN=0
TESTS_PASSED=0
TESTS_FAILED=0

# Project root (parent of tests directory)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
DRY="$PROJECT_ROOT/.build/dry"

# Test temp directory
TEST_TMP="$SCRIPT_DIR/tmp"

# Cleanup function
cleanup() {
    rm -rf "$TEST_TMP"
}
trap cleanup EXIT

# Initialize test environment
init_tests() {
    echo "========================================"
    echo "  DRY Diary Test Suite"
    echo "========================================"
    echo ""
    
    # Build the project first
    echo -n "Building project... "
    if make -C "$PROJECT_ROOT" -s > /dev/null 2>&1; then
        echo -e "${GREEN}OK${NC}"
    else
        echo -e "${RED}FAILED${NC}"
        echo "Build failed. Run 'make' to see errors."
        exit 1
    fi
    
    # Check binary exists
    if [[ ! -x "$DRY" ]]; then
        echo -e "${RED}Error: Binary not found at $DRY${NC}"
        exit 1
    fi
    
    # Create temp directory
    mkdir -p "$TEST_TMP"
    
    echo ""
}

# Test assertion helpers
assert_exit_code() {
    local expected="$1"
    local actual="$2"
    local msg="$3"
    
    if [[ "$actual" -eq "$expected" ]]; then
        return 0
    else
        echo "  Expected exit code $expected, got $actual"
        return 1
    fi
}

assert_output_contains() {
    local needle="$1"
    local haystack="$2"
    
    if echo "$haystack" | grep -q "$needle"; then
        return 0
    else
        echo "  Expected output to contain: $needle"
        echo "  Actual output: $haystack"
        return 1
    fi
}

assert_output_not_contains() {
    local needle="$1"
    local haystack="$2"
    
    if ! echo "$haystack" | grep -q "$needle"; then
        return 0
    else
        echo "  Expected output NOT to contain: $needle"
        return 1
    fi
}

# Run a single test
run_test() {
    local name="$1"
    local test_func="$2"
    
    TESTS_RUN=$((TESTS_RUN + 1))
    echo -n "  $name... "
    
    if $test_func > "$TEST_TMP/test_output.txt" 2>&1; then
        echo -e "${GREEN}PASS${NC}"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}FAIL${NC}"
        TESTS_FAILED=$((TESTS_FAILED + 1))
        # Show failure details
        cat "$TEST_TMP/test_output.txt" | sed 's/^/    /'
    fi
}

# =============================================================================
# TEST CASES: Help and Version
# =============================================================================

test_help_short() {
    local output
    output=$("$DRY" -h 2>&1)
    local rc=$?
    
    assert_exit_code 0 $rc "exit code" &&
    assert_output_contains "Usage:" "$output" &&
    assert_output_contains "COMMANDS" "$output" &&
    assert_output_contains "OPTIONS" "$output"
}

test_help_long() {
    local output
    output=$("$DRY" --help 2>&1)
    local rc=$?
    
    assert_exit_code 0 $rc "exit code" &&
    assert_output_contains "Usage:" "$output" &&
    assert_output_contains "init" "$output" &&
    assert_output_contains "new" "$output" &&
    assert_output_contains "list" "$output" &&
    assert_output_contains "show" "$output" &&
    assert_output_contains "delete" "$output" &&
    assert_output_contains "explore" "$output"
}

test_version_short() {
    local output
    output=$("$DRY" -v 2>&1)
    local rc=$?
    
    assert_exit_code 0 $rc "exit code" &&
    assert_output_contains "version" "$output"
}

test_version_long() {
    local output
    output=$("$DRY" --version 2>&1)
    local rc=$?
    
    assert_exit_code 0 $rc "exit code" &&
    assert_output_contains "version" "$output"
}

test_no_args_shows_help() {
    local output
    output=$("$DRY" 2>&1)
    local rc=$?
    
    assert_exit_code 1 $rc "exit code" &&
    assert_output_contains "Usage:" "$output"
}

# =============================================================================
# TEST CASES: Subcommand Help
# =============================================================================

test_init_help() {
    local output
    output=$("$DRY" init --help 2>&1)
    local rc=$?
    
    assert_exit_code 0 $rc "exit code" &&
    assert_output_contains "Initialize a new diary" "$output" &&
    assert_output_contains "<name>" "$output"
}

test_init_help_short() {
    local output
    output=$("$DRY" init -h 2>&1)
    local rc=$?
    
    assert_exit_code 0 $rc "exit code" &&
    assert_output_contains "Initialize a new diary" "$output"
}

test_new_help() {
    local output
    output=$("$DRY" new --help 2>&1)
    local rc=$?
    
    assert_exit_code 0 $rc "exit code" &&
    assert_output_contains "Add a new entry" "$output" &&
    assert_output_contains "video" "$output" &&
    assert_output_contains "note" "$output"
}

test_list_help() {
    local output
    output=$("$DRY" list --help 2>&1)
    local rc=$?
    
    assert_exit_code 0 $rc "exit code" &&
    assert_output_contains "List diary entries" "$output" &&
    assert_output_contains "today" "$output" &&
    assert_output_contains "yesterday" "$output"
}

test_show_help() {
    local output
    output=$("$DRY" show --help 2>&1)
    local rc=$?
    
    assert_exit_code 0 $rc "exit code" &&
    assert_output_contains "Show an entry" "$output" &&
    assert_output_contains "<id>" "$output"
}

test_delete_help() {
    local output
    output=$("$DRY" delete --help 2>&1)
    local rc=$?
    
    assert_exit_code 0 $rc "exit code" &&
    assert_output_contains "Delete an entry" "$output" &&
    assert_output_contains "<id>" "$output"
}

test_explore_help() {
    local output
    output=$("$DRY" explore --help 2>&1)
    local rc=$?
    
    assert_exit_code 0 $rc "exit code" &&
    assert_output_contains "file manager" "$output"
}

# Help flag after positional argument
test_list_arg_then_help() {
    local output
    output=$("$DRY" list today --help 2>&1)
    local rc=$?
    
    assert_exit_code 0 $rc "exit code" &&
    assert_output_contains "List diary entries" "$output"
}

# =============================================================================
# TEST CASES: Argument Validation Errors
# =============================================================================

test_unknown_command() {
    local output
    output=$("$DRY" foobar 2>&1)
    local rc=$?
    
    assert_exit_code 1 $rc "exit code" &&
    assert_output_contains "unknown command" "$output"
}

test_init_missing_name() {
    local output
    output=$("$DRY" init 2>&1)
    local rc=$?
    
    assert_exit_code 1 $rc "exit code" &&
    assert_output_contains "Error" "$output"
}

test_new_missing_type() {
    local output
    output=$("$DRY" new 2>&1)
    local rc=$?
    
    assert_exit_code 1 $rc "exit code" &&
    assert_output_contains "Error" "$output"
}

test_new_invalid_type() {
    local output
    output=$("$DRY" new invalid 2>&1)
    local rc=$?
    
    assert_output_contains "wrong type" "$output" ||
    assert_output_contains "video" "$output"
}

test_show_missing_id() {
    local output
    output=$("$DRY" show 2>&1)
    local rc=$?
    
    assert_exit_code 1 $rc "exit code" &&
    assert_output_contains "Error" "$output"
}

test_delete_missing_diary() {
    local output
    output=$("$DRY" delete someid 2>&1)
    local rc=$?
    
    assert_exit_code 1 $rc "exit code" &&
    assert_output_contains "requires -d" "$output"
}

test_delete_missing_id() {
    local output
    output=$("$DRY" -d test delete 2>&1)
    local rc=$?
    
    assert_exit_code 1 $rc "exit code" &&
    assert_output_contains "Error" "$output"
}

test_list_too_many_args() {
    local output
    output=$("$DRY" list arg1 arg2 2>&1)
    local rc=$?
    
    assert_exit_code 1 $rc "exit code" &&
    assert_output_contains "too many" "$output"
}

# =============================================================================
# TEST CASES: Option Parsing
# =============================================================================

test_diary_option_short_before() {
    # Should accept -d before subcommand
    local output
    output=$("$DRY" -d testdiary list --help 2>&1)
    local rc=$?
    
    assert_exit_code 0 $rc "exit code"
}

test_diary_option_long_before() {
    local output
    output=$("$DRY" --diary testdiary list --help 2>&1)
    local rc=$?
    
    assert_exit_code 0 $rc "exit code"
}

test_diary_option_after_subcommand() {
    local output
    output=$("$DRY" list -d testdiary --help 2>&1)
    local rc=$?
    
    assert_exit_code 0 $rc "exit code"
}

test_diary_option_equals_form() {
    local output
    output=$("$DRY" --diary=testdiary list --help 2>&1)
    local rc=$?
    
    assert_exit_code 0 $rc "exit code"
}

test_diary_option_combined() {
    # -dtestdiary form
    local output
    output=$("$DRY" -dtestdiary list --help 2>&1)
    local rc=$?
    
    assert_exit_code 0 $rc "exit code"
}

# =============================================================================
# TEST CASES: Program Name in Usage
# =============================================================================

test_usage_shows_program_name() {
    local output
    output=$("$DRY" new 2>&1)
    
    # Should show the actual binary name/path in usage
    assert_output_contains "dry" "$output" ||
    assert_output_contains "Usage:" "$output"
}

# =============================================================================
# Run all tests
# =============================================================================

run_test_suite() {
    local suite_name="$1"
    shift
    
    echo ""
    echo "[$suite_name]"
    
    for test_func in "$@"; do
        local test_name="${test_func#test_}"
        test_name="${test_name//_/ }"
        run_test "$test_name" "$test_func"
    done
}

main() {
    init_tests
    
    run_test_suite "Help and Version" \
        test_help_short \
        test_help_long \
        test_version_short \
        test_version_long \
        test_no_args_shows_help
    
    run_test_suite "Subcommand Help" \
        test_init_help \
        test_init_help_short \
        test_new_help \
        test_list_help \
        test_show_help \
        test_delete_help \
        test_explore_help \
        test_list_arg_then_help
    
    run_test_suite "Argument Validation" \
        test_unknown_command \
        test_init_missing_name \
        test_new_missing_type \
        test_new_invalid_type \
        test_show_missing_id \
        test_delete_missing_diary \
        test_delete_missing_id \
        test_list_too_many_args
    
    run_test_suite "Option Parsing" \
        test_diary_option_short_before \
        test_diary_option_long_before \
        test_diary_option_after_subcommand \
        test_diary_option_equals_form \
        test_diary_option_combined
    
    run_test_suite "Miscellaneous" \
        test_usage_shows_program_name
    
    # Summary
    echo ""
    echo "========================================"
    echo "  Results: $TESTS_PASSED/$TESTS_RUN passed"
    if [[ $TESTS_FAILED -gt 0 ]]; then
        echo -e "  ${RED}$TESTS_FAILED test(s) failed${NC}"
        echo "========================================"
        exit 1
    else
        echo -e "  ${GREEN}All tests passed!${NC}"
        echo "========================================"
        exit 0
    fi
}

main "$@"
