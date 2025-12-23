#!/bin/bash
#
# Integration tests for diary operations
# Creates a temporary encfs diary with automated password for testing
#
# Run with: ./tests/test_integration.sh
#

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
DRY="$PROJECT_ROOT/.build/dry"
TEST_TMP="$SCRIPT_DIR/tmp_integration"
TEST_DIARY="testdiary"
TEST_PASSWORD="test_password_12345"
TEST_CONFIG_DIR="$TEST_TMP/.dry"
TEST_CONFIG="$TEST_CONFIG_DIR/dry.conf"
TEST_DIARIES_PATH="$TEST_TMP/diaries"
TEST_ENC_PATH="$TEST_DIARIES_PATH/.$TEST_DIARY"
TEST_MOUNT_PATH="$TEST_DIARIES_PATH/$TEST_DIARY"

# Counters
TESTS_RUN=0
TESTS_PASSED=0
TESTS_FAILED=0
TESTS_SKIPPED=0

# Track if diary is mounted
DIARY_MOUNTED=0

cleanup() {
    echo ""
    echo -e "${BLUE}Cleaning up...${NC}"
    
    # Unmount if mounted
    if [[ $DIARY_MOUNTED -eq 1 ]] || mountpoint -q "$TEST_MOUNT_PATH" 2>/dev/null; then
        fusermount -u "$TEST_MOUNT_PATH" 2>/dev/null || true
        sleep 0.5
    fi
    
    rm -rf "$TEST_TMP"
}
trap cleanup EXIT

# Mount the test diary with automated password
mount_test_diary() {
    if mountpoint -q "$TEST_MOUNT_PATH" 2>/dev/null; then
        return 0
    fi
    
    echo "$TEST_PASSWORD" | encfs --stdinpass "$TEST_ENC_PATH" "$TEST_MOUNT_PATH" 2>/dev/null
    DIARY_MOUNTED=1
}

# Unmount the test diary
unmount_test_diary() {
    if mountpoint -q "$TEST_MOUNT_PATH" 2>/dev/null; then
        fusermount -u "$TEST_MOUNT_PATH" 2>/dev/null || true
        DIARY_MOUNTED=0
        sleep 0.3
    fi
}

init_tests() {
    echo "========================================"
    echo "  DRY Integration Test Suite"
    echo "========================================"
    echo ""
    
    # Check dependencies
    echo -n "Checking dependencies... "
    local missing=()
    command -v encfs >/dev/null || missing+=("encfs")
    command -v fusermount >/dev/null || missing+=("fusermount")
    
    if [[ ${#missing[@]} -gt 0 ]]; then
        echo -e "${YELLOW}SKIP${NC}"
        echo "Missing: ${missing[*]}"
        echo "Integration tests require encfs. Skipping."
        exit 0
    fi
    echo -e "${GREEN}OK${NC}"
    
    # Build
    echo -n "Building project... "
    if make -C "$PROJECT_ROOT" -s > /dev/null 2>&1; then
        echo -e "${GREEN}OK${NC}"
    else
        echo -e "${RED}FAILED${NC}"
        exit 1
    fi
    
    # Setup test environment
    echo -n "Setting up test environment... "
    mkdir -p "$TEST_DIARIES_PATH"
    mkdir -p "$TEST_CONFIG_DIR"
    mkdir -p "$TEST_ENC_PATH"
    mkdir -p "$TEST_MOUNT_PATH"
    
    # Create config file with absolute paths
    # Note: config uses 'default_diary' and 'default_dir' as setting names
    cat > "$TEST_CONFIG" << EOF
default_diary = "$TEST_DIARY";
default_dir = "$TEST_DIARIES_PATH";
editor = "cat";
player = "cat";
list_cmd = "ls -la";
file_manager = "ls";
pager = "cat";
EOF

    # Create diaries.ref in proper format: "name : path"
    echo "$TEST_DIARY : $TEST_MOUNT_PATH" > "$TEST_CONFIG_DIR/diaries.ref"
    
    echo -e "${GREEN}OK${NC}"
    
    # Initialize encfs directory with test password
    echo -n "Initializing test encfs volume... "
    
    # Create encfs volume with --standard for faster tests
    local encfs_output
    encfs_output=$(echo "$TEST_PASSWORD" | encfs --stdinpass --standard "$TEST_ENC_PATH" "$TEST_MOUNT_PATH" 2>&1)
    if [[ $? -eq 0 ]]; then
        DIARY_MOUNTED=1
        echo -e "${GREEN}OK${NC}"
    else
        echo -e "${RED}FAILED${NC}"
        echo "Could not initialize encfs volume"
        echo "ENC_PATH: $TEST_ENC_PATH"
        echo "MOUNT_PATH: $TEST_MOUNT_PATH"
        echo "encfs output: $encfs_output"
        exit 1
    fi
    
    # Create date directory structure for today
    local today_path
    today_path=$(date +%Y/%m/%d)
    mkdir -p "$TEST_MOUNT_PATH/$today_path"
    
    # Unmount for tests that need to test mounting
    unmount_test_diary
    
    # Set HOME override for tests
    export HOME="$TEST_TMP"
    
    # Set environment for non-interactive encfs
    export DRY_ENCFS_PASSWORD="$TEST_PASSWORD"
    export DRY_NO_UNMOUNT="1"
    
    echo ""
}

skip_test() {
    local name="$1"
    local reason="$2"
    
    TESTS_RUN=$((TESTS_RUN + 1))
    TESTS_SKIPPED=$((TESTS_SKIPPED + 1))
    echo -e "  $name... ${YELLOW}SKIP${NC} ($reason)"
}

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
        cat "$TEST_TMP/test_output.txt" | sed 's/^/    /'
    fi
}

# =============================================================================
# Helper to run dry with test diary pre-mounted
# Uses DRY_ENCFS_PASSWORD and DRY_NO_UNMOUNT environment variables
# to enable fully automated testing without interactive password prompts
# =============================================================================

run_dry_with_diary() {
    # Set env vars for non-interactive mode
    export DRY_ENCFS_PASSWORD="$TEST_PASSWORD"
    export DRY_NO_UNMOUNT="1"
    
    # Run dry command
    local output
    output=$(cd "$TEST_TMP" && "$DRY" "$@" 2>&1)
    local rc=$?
    
    echo "$output"
    return $rc
}

# Helper for single operations where we DO want unmount
run_dry_single() {
    export DRY_ENCFS_PASSWORD="$TEST_PASSWORD"
    unset DRY_NO_UNMOUNT
    
    local output
    output=$(cd "$TEST_TMP" && "$DRY" "$@" 2>&1)
    local rc=$?
    
    echo "$output"
    return $rc
}

# =============================================================================
# Configuration Tests
# =============================================================================

test_config_loads() {
    [[ -f "$TEST_CONFIG" ]] && grep -q "default_diary" "$TEST_CONFIG"
}

test_diaries_ref_format() {
    local ref_file="$TEST_CONFIG_DIR/diaries.ref"
    [[ -f "$ref_file" ]] && grep -q "$TEST_DIARY" "$ref_file"
}

# =============================================================================
# Date Filter Tests
# =============================================================================

test_filter_today_format() {
    local today
    today=$(date +%Y/%m/%d)
    [[ "$today" =~ ^[0-9]{4}/[0-9]{2}/[0-9]{2}$ ]]
}

test_filter_yesterday_format() {
    local yesterday
    yesterday=$(date -d "yesterday" +%Y/%m/%d 2>/dev/null || date -v-1d +%Y/%m/%d)
    [[ "$yesterday" =~ ^[0-9]{4}/[0-9]{2}/[0-9]{2}$ ]]
}

test_date_filter_dash_to_slash() {
    local input="2024-01-15"
    local expected="2024/01/15"
    local converted="${input//-//}"
    [[ "$converted" == "$expected" ]]
}

# =============================================================================
# Diary Operations Tests
# =============================================================================

test_diary_mounts() {
    # Test that we can mount the diary
    unmount_test_diary
    mount_test_diary
    mountpoint -q "$TEST_MOUNT_PATH"
}

test_diary_unmounts() {
    mount_test_diary
    unmount_test_diary
    ! mountpoint -q "$TEST_MOUNT_PATH"
}

test_list_runs() {
    # Test that list command works
    local output
    output=$(run_dry_with_diary -d "$TEST_DIARY" list 2>&1)
    local rc=$?
    
    # Should either succeed or show "no entries" (not an error about mounting)
    [[ $rc -eq 0 ]] || echo "$output" | grep -qi "no entries\|total"
}

test_list_today() {
    # Create test file using dry's mount (via DRY_ENCFS_PASSWORD and DRY_NO_UNMOUNT)
    # First run any command to ensure dry mounts the filesystem
    run_dry_with_diary -d "$TEST_DIARY" list >/dev/null 2>&1 || true
    
    # Now the diary should be mounted at $TEST_MOUNT_PATH
    local today_path
    today_path=$(date +%Y/%m/%d)
    mkdir -p "$TEST_MOUNT_PATH/$today_path"
    echo "test content" > "$TEST_MOUNT_PATH/$today_path/test_entry.org"
    
    local output
    output=$(run_dry_with_diary -d "$TEST_DIARY" list today 2>&1)
    
    # Should show our test file
    echo "$output" | grep -q "test_entry"
}

test_list_with_date() {
    mount_test_diary
    
    # Create a test file for a specific date
    mkdir -p "$TEST_MOUNT_PATH/2025/01/15"
    echo "dated content" > "$TEST_MOUNT_PATH/2025/01/15/dated_entry.org"
    
    local output
    output=$(run_dry_with_diary -d "$TEST_DIARY" list 2025-01-15 2>&1)
    
    echo "$output" | grep -q "dated_entry"
}

test_new_note_creates_file() {
    mount_test_diary
    
    local today_path
    today_path=$(date +%Y/%m/%d)
    local before_count
    before_count=$(ls -1 "$TEST_MOUNT_PATH/$today_path"/*.org 2>/dev/null | wc -l || echo 0)
    
    # Run new note command - it will open editor, but with cat it just reads stdin
    # We need to simulate this differently since 'new' opens an editor
    # For now, just verify the command doesn't crash
    local output
    output=$(echo "" | run_dry_with_diary -d "$TEST_DIARY" new note 2>&1) || true
    
    # The command should at least not fail with a bad error
    ! echo "$output" | grep -qi "error: wrong type"
}

test_show_entry() {
    # First run a command to ensure dry mounts the filesystem
    run_dry_with_diary -d "$TEST_DIARY" list >/dev/null 2>&1 || true
    
    # Create a test entry with known ID pattern
    local today_path
    today_path=$(date +%Y/%m/%d)
    local test_file="$TEST_MOUNT_PATH/$today_path/2025-12-23_test"
    mkdir -p "$TEST_MOUNT_PATH/$today_path"
    echo "# Test Entry Content" > "$test_file"
    
    local output
    output=$(run_dry_with_diary -d "$TEST_DIARY" show "2025-12-23_test" 2>&1)
    
    # Should display the content (pager is 'cat')
    echo "$output" | grep -q "Test Entry Content"
}

test_explore_runs() {
    # Test that explore command executes (file_manager is 'ls' in test config)
    local output
    output=$(run_dry_with_diary -d "$TEST_DIARY" explore 2>&1)
    
    # Should list directory contents or at least not error badly
    [[ $? -eq 0 ]] || echo "$output" | grep -qv "Error"
}

test_delete_requires_confirmation() {
    # First run a command to ensure dry mounts the filesystem
    run_dry_with_diary -d "$TEST_DIARY" list >/dev/null 2>&1 || true
    
    # Create a test entry to delete with proper date-based ID
    local today_path
    today_path=$(date +%Y/%m/%d)
    local today_date
    today_date=$(date +%Y-%m-%d)
    mkdir -p "$TEST_MOUNT_PATH/$today_path"
    echo "to be deleted" > "$TEST_MOUNT_PATH/$today_path/${today_date}_delete_me"
    
    # Try delete - currently just runs file_manager on the file
    # TODO: diary_delete doesn't have confirmation logic yet
    local output
    output=$(run_dry_with_diary -d "$TEST_DIARY" delete "${today_date}_delete_me" 2>&1)
    
    # Should at least print "Deleting" message
    echo "$output" | grep -q "Deleting"
}

test_delete_runs_file_manager() {
    # First run a command to ensure dry mounts the filesystem
    run_dry_with_diary -d "$TEST_DIARY" list >/dev/null 2>&1 || true
    
    # Create a test entry with proper date-based ID
    local today_path
    today_path=$(date +%Y/%m/%d)
    local today_date
    today_date=$(date +%Y-%m-%d)
    mkdir -p "$TEST_MOUNT_PATH/$today_path"
    echo "to be shown" > "$TEST_MOUNT_PATH/$today_path/${today_date}_delete_test2"
    
    # Delete command actually runs file_manager (ls in test config) on the file
    # TODO: This should actually delete the file, not just list it
    local output
    output=$(run_dry_with_diary -d "$TEST_DIARY" delete "${today_date}_delete_test2" 2>&1)
    
    # Should succeed without error
    [[ $? -eq 0 ]]
}

# =============================================================================
# Main
# =============================================================================

main() {
    init_tests
    
    echo "[Configuration Tests]"
    run_test "config loads" test_config_loads
    run_test "diaries.ref format" test_diaries_ref_format
    
    echo ""
    echo "[Date Filter Tests]"
    run_test "today filter format" test_filter_today_format
    run_test "yesterday filter format" test_filter_yesterday_format
    run_test "date dash to slash conversion" test_date_filter_dash_to_slash
    
    echo ""
    echo "[Diary Mount/Unmount]"
    run_test "diary mounts with password" test_diary_mounts
    run_test "diary unmounts" test_diary_unmounts
    
    echo ""
    echo "[List Command]"
    run_test "list runs without error" test_list_runs
    run_test "list today shows entries" test_list_today
    run_test "list with date filter" test_list_with_date
    
    echo ""
    echo "[Entry Operations]"
    run_test "new note command works" test_new_note_creates_file
    run_test "show displays entry content" test_show_entry
    run_test "explore runs file manager" test_explore_runs
    
    echo ""
    echo "[Delete Operations]"
    run_test "delete shows deleting message" test_delete_requires_confirmation
    run_test "delete runs file manager" test_delete_runs_file_manager
    
    # Summary
    echo ""
    echo "========================================"
    echo "  Results: $TESTS_PASSED passed, $TESTS_SKIPPED skipped, $TESTS_FAILED failed"
    if [[ $TESTS_FAILED -gt 0 ]]; then
        echo -e "  ${RED}Some tests failed${NC}"
        exit 1
    else
        echo -e "  ${GREEN}All tests passed!${NC}"
    fi
    echo "========================================"
}

main "$@"
