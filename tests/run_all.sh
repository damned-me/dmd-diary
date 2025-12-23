#!/bin/bash
#
# Run all test suites
#
# Usage:
#   ./tests/run_all.sh          # Run all tests
#   ./tests/run_all.sh quick    # Run only quick CLI tests
#   ./tests/run_all.sh verbose  # Run with verbose output
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

TOTAL_SUITES=0
PASSED_SUITES=0
FAILED_SUITES=0

run_suite() {
    local name="$1"
    local script="$2"
    
    TOTAL_SUITES=$((TOTAL_SUITES + 1))
    
    echo ""
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${BLUE}  Running: $name${NC}"
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    
    if [[ -x "$script" ]]; then
        if "$script"; then
            PASSED_SUITES=$((PASSED_SUITES + 1))
        else
            FAILED_SUITES=$((FAILED_SUITES + 1))
        fi
    else
        echo -e "${YELLOW}Script not executable: $script${NC}"
        FAILED_SUITES=$((FAILED_SUITES + 1))
    fi
}

main() {
    local mode="${1:-all}"
    
    echo ""
    echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
    echo -e "${BLUE}║       DRY DIARY TEST RUNNER            ║${NC}"
    echo -e "${BLUE}╚════════════════════════════════════════╝${NC}"
    
    # Make test scripts executable
    chmod +x "$SCRIPT_DIR"/*.sh 2>/dev/null || true
    
    case "$mode" in
        quick)
            run_suite "CLI Tests" "$SCRIPT_DIR/test_runner.sh"
            ;;
        completion)
            run_suite "Completion Tests" "$SCRIPT_DIR/test_completion.sh"
            ;;
        integration)
            run_suite "Integration Tests" "$SCRIPT_DIR/test_integration.sh"
            ;;
        all|*)
            run_suite "CLI Tests" "$SCRIPT_DIR/test_runner.sh"
            run_suite "Completion Tests" "$SCRIPT_DIR/test_completion.sh"
            run_suite "Integration Tests" "$SCRIPT_DIR/test_integration.sh"
            ;;
    esac
    
    # Final summary
    echo ""
    echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
    echo -e "${BLUE}║            FINAL SUMMARY               ║${NC}"
    echo -e "${BLUE}╚════════════════════════════════════════╝${NC}"
    echo ""
    echo "  Test Suites: $PASSED_SUITES/$TOTAL_SUITES passed"
    
    if [[ $FAILED_SUITES -gt 0 ]]; then
        echo -e "  ${RED}$FAILED_SUITES suite(s) had failures${NC}"
        echo ""
        exit 1
    else
        echo -e "  ${GREEN}All test suites passed!${NC}"
        echo ""
        exit 0
    fi
}

main "$@"
