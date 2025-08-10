#!/bin/bash

echo "======================================"
echo "    Darknet Test Suite Runner"
echo "======================================"
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Build main project first
echo -e "${YELLOW}Building main project...${NC}"
cd ..
make clean > /dev/null 2>&1
make -j4
if [ $? -ne 0 ]; then
    echo -e "${RED}Failed to build main project${NC}"
    exit 1
fi
echo -e "${GREEN}Main project built successfully${NC}"
echo ""

# Build tests
cd tests
echo -e "${YELLOW}Building test suite...${NC}"
make clean > /dev/null 2>&1
make -j4
if [ $? -ne 0 ]; then
    echo -e "${RED}Failed to build tests${NC}"
    exit 1
fi
echo -e "${GREEN}Tests built successfully${NC}"
echo ""

# Run tests
echo -e "${YELLOW}Running tests...${NC}"
echo ""

TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Array of test executables
TESTS=(test_utils test_data test_network test_box test_image test_blas)

for test in "${TESTS[@]}"; do
    echo "----------------------------------------"
    echo "Running: $test"
    echo "----------------------------------------"
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    if ./$test; then
        echo -e "${GREEN}✓ $test passed${NC}"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        echo -e "${RED}✗ $test failed${NC}"
        FAILED_TESTS=$((FAILED_TESTS + 1))
    fi
    echo ""
done

# Summary
echo "======================================"
echo "           Test Summary"
echo "======================================"
echo -e "Total Tests:  $TOTAL_TESTS"
echo -e "Passed:       ${GREEN}$PASSED_TESTS${NC}"
echo -e "Failed:       ${RED}$FAILED_TESTS${NC}"

if [ $FAILED_TESTS -eq 0 ]; then
    echo ""
    echo -e "${GREEN}All tests passed successfully!${NC}"
    exit 0
else
    echo ""
    echo -e "${RED}Some tests failed. Please review the output above.${NC}"
    exit 1
fi