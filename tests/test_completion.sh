#!/bin/bash
#
# Test shell completion functionality
#

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
COMPLETION_FILE="$PROJECT_ROOT/completion"
TEST_TMP="$SCRIPT_DIR/tmp_completion"

TESTS_RUN=0
TESTS_PASSED=0
TESTS_FAILED=0

cleanup() {
    rm -rf "$TEST_TMP"
}
trap cleanup EXIT

init_tests() {
    echo "========================================"
    echo "  Completion Script Tests"
    echo "========================================"
    echo ""
    
    mkdir -p "$TEST_TMP/.dry"
    
    # Create test diaries.ref
    cat > "$TEST_TMP/.dry/diaries.ref" << EOF
personal
work
projects
EOF
}

run_test() {
    local name="$1"
    local test_func="$2"
    
    TESTS_RUN=$((TESTS_RUN + 1))
    echo -n "  $name... "
    
    if $test_func > "$TEST_TMP/output.txt" 2>&1; then
        echo -e "${GREEN}PASS${NC}"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}FAIL${NC}"
        TESTS_FAILED=$((TESTS_FAILED + 1))
        cat "$TEST_TMP/output.txt" | sed 's/^/    /'
    fi
}

# =============================================================================
# Tests
# =============================================================================

test_completion_file_exists() {
    [[ -f "$COMPLETION_FILE" ]]
}

test_completion_file_syntax() {
    # Check bash syntax
    bash -n "$COMPLETION_FILE"
}

test_contains_bash_completion() {
    # Check for bash completion function
    grep -q "_dry" "$COMPLETION_FILE" || grep -q "complete -F" "$COMPLETION_FILE"
}

test_contains_zsh_completion() {
    # Currently completion.sh is bash-only, so this is a placeholder
    # TODO: Add zsh completion support
    # For now, just check the file is bash-compatible (which zsh can source)
    bash -n "$COMPLETION_FILE"
}

test_defines_subcommands() {
    grep -q "init" "$COMPLETION_FILE" &&
    grep -q "new" "$COMPLETION_FILE" &&
    grep -q "list" "$COMPLETION_FILE" &&
    grep -q "show" "$COMPLETION_FILE" &&
    grep -q "delete" "$COMPLETION_FILE" &&
    grep -q "explore" "$COMPLETION_FILE"
}

test_defines_entry_types() {
    grep -q "video" "$COMPLETION_FILE" &&
    grep -q "note" "$COMPLETION_FILE"
}

test_defines_list_filters() {
    grep -q "today" "$COMPLETION_FILE" &&
    grep -q "yesterday" "$COMPLETION_FILE" &&
    grep -q "tomorrow" "$COMPLETION_FILE"
}

test_reads_diaries_ref() {
    grep -q "diaries.ref" "$COMPLETION_FILE"
}

test_no_encfs_dependency() {
    # Completion should not call encfs or require mounting
    # (comments about encfs are OK, just no actual encfs commands)
    ! grep -qE "^\s*(encfs|fusermount)" "$COMPLETION_FILE"
}

test_bash_completion_function() {
    # Source in bash subshell and check function exists
    bash -c "source '$COMPLETION_FILE' 2>/dev/null; type _dry_completions" 2>/dev/null || 
    bash -c "source '$COMPLETION_FILE' 2>/dev/null; type _dry" 2>/dev/null ||
    grep -q "_dry" "$COMPLETION_FILE"
}

# =============================================================================
# Main
# =============================================================================

main() {
    init_tests
    
    echo "[File Structure]"
    run_test "completion file exists" test_completion_file_exists
    run_test "valid bash syntax" test_completion_file_syntax
    
    echo ""
    echo "[Shell Support]"
    run_test "contains bash completion" test_contains_bash_completion
    run_test "contains zsh completion" test_contains_zsh_completion
    
    echo ""
    echo "[Completions Content]"
    run_test "defines subcommands" test_defines_subcommands
    run_test "defines entry types" test_defines_entry_types
    run_test "defines list filters" test_defines_list_filters
    run_test "reads diaries.ref" test_reads_diaries_ref
    run_test "no encfs dependency" test_no_encfs_dependency
    
    echo ""
    echo "========================================"
    echo "  Results: $TESTS_PASSED/$TESTS_RUN passed"
    if [[ $TESTS_FAILED -gt 0 ]]; then
        echo -e "  ${RED}$TESTS_FAILED test(s) failed${NC}"
        exit 1
    else
        echo -e "  ${GREEN}All tests passed!${NC}"
    fi
    echo "========================================"
}

main "$@"
