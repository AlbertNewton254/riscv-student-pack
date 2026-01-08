#!/bin/bash

# ANSI Colors for output (ASCII safe)
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Configuration
BUILD_DIRS=("../assembler" "../emulator")
BINARIES=("../assembler/riscv_assembler" "../emulator/riscv_emulator")
INTEGRATION_TEST="test_integration"

# Clean up
cleanup() {
    echo "Cleaning object files and test executables..."
    for dir in "${BUILD_DIRS[@]}"; do
        make -C "$dir" clean 2>/dev/null || true
    done

    # Remove test files
    make -C "." clean 2>/dev/null || true
    rm -f *.bin *.s 2>/dev/null || true
}

# Build project
build_project() {
    local status=0

    for dir in "${BUILD_DIRS[@]}"; do
        echo -e "\n${YELLOW}--- Building $dir ---${NC}"
        if ! make -C "$dir" all; then
            echo -e "${RED}Build failed for $dir${NC}"
            make -C "$dir" all  # Run again without redirect to show errors
            status=1
        else
            echo -e "${GREEN}[PASS] $dir build${NC}"
        fi
    done

    return $status
}

# Run integration tests
run_integration_tests() {
    local status=0

    echo -e "\n${YELLOW}--- Running integration tests ---${NC}"
    if ! make -C "." test_integration; then
        echo -e "${RED}[FAIL] Integration tests${NC}"
        status=1
    else
        echo -e "${GREEN}[PASS] Integration tests${NC}"
    fi

    return $status
}

# Main execution
main() {
    echo -e "${YELLOW}=== RISC-V Integration Tests ===${NC}"

    # Initial cleanup
    cleanup

    # Build project
    if ! build_project; then
        echo -e "\n${RED}Build failed! Cannot run tests.${NC}"
        exit 1
    fi

    # Build and run integration tests
    test_status=0
    run_integration_tests || test_status=1

    # Final cleanup (keep binaries)
    echo -e "\n${YELLOW}--- Final cleanup (keeping binaries) ---${NC}"
    find . -name "*.o" -delete 2>/dev/null || true
    rm -f "$INTEGRATION_TEST" 2>/dev/null || true
    rm -f *.bin *.s 2>/dev/null || true

    # Summary
    echo -e "\n${YELLOW}=== Summary ===${NC}"
    echo -e "${YELLOW}Binaries preserved:${NC}"
    for binary in "${BINARIES[@]}"; do
        if [ -f "$binary" ]; then
            echo "  ./$binary"
        fi
    done

    # Overall status
    if [ $test_status -eq 0 ]; then
        echo -e "\n${GREEN}=== All integration tests passed! ===${NC}"
        exit 0
    else
        echo -e "\n${RED}=== Some integration tests failed ===${NC}"
        exit 1
    fi
}

# Run main function
main
