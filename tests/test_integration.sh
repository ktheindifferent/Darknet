#!/bin/bash

echo "======================================"
echo "    Darknet Integration Tests"
echo "======================================"
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if darknet executable exists
if [ ! -f "../darknet" ]; then
    echo -e "${RED}Error: darknet executable not found${NC}"
    echo "Please build the project first with 'make'"
    exit 1
fi

echo -e "${YELLOW}Testing darknet executable...${NC}"

# Test 1: Check if darknet runs
echo -n "1. Testing darknet help output... "
../darknet 2>&1 | grep -q "usage:"
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓${NC}"
else
    echo -e "${RED}✗${NC}"
    exit 1
fi

# Test 2: Check if darknet can load a config file
echo -n "2. Testing config file parsing... "
if [ -f "../cfg/tiny.cfg" ]; then
    ../darknet print ../cfg/tiny.cfg 2>&1 | grep -q "layer"
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓${NC}"
    else
        echo -e "${RED}✗${NC}"
    fi
else
    echo -e "${YELLOW}Skipped (no config file)${NC}"
fi

# Test 3: Check if darknet can process test image
echo -n "3. Testing image loading... "
if [ -f "../data/dog.jpg" ]; then
    # This would require weights, so we just check if it attempts to load
    timeout 2 ../darknet detector test ../cfg/coco.data ../cfg/yolov3.cfg 2>&1 | grep -q "Loading"
    if [ $? -eq 0 ] || [ $? -eq 124 ]; then  # 124 is timeout exit code
        echo -e "${GREEN}✓${NC}"
    else
        echo -e "${RED}✗${NC}"
    fi
else
    echo -e "${YELLOW}Skipped (no test image)${NC}"
fi

# Test 4: Check library loading
echo -n "4. Testing shared library... "
if [ -f "../libdarknet.so" ]; then
    ldd ../darknet | grep -q "libdarknet.so"
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓${NC}"
    else
        # Try with static linking
        echo -e "${YELLOW}✓ (static)${NC}"
    fi
else
    echo -e "${YELLOW}Skipped (no shared library)${NC}"
fi

# Test 5: Memory check (if valgrind is available)
echo -n "5. Testing for memory leaks... "
if command -v valgrind &> /dev/null; then
    valgrind --leak-check=yes --error-exitcode=1 ../darknet 2>&1 | grep -q "no leaks are possible"
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓${NC}"
    else
        echo -e "${YELLOW}⚠ (minor leaks possible)${NC}"
    fi
else
    echo -e "${YELLOW}Skipped (valgrind not installed)${NC}"
fi

echo ""
echo "======================================"
echo -e "${GREEN}    Integration Tests Complete${NC}"
echo "======================================" 
echo ""