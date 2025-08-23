#!/bin/bash

# Thread Safety Test Suite Script
# Tests thread synchronization implementation in Darknet

set -e

echo "========================================"
echo "  Thread Safety Test Suite for Darknet  "
echo "========================================"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Compilation flags
CFLAGS="-Wall -Wno-unused-result -Wno-unknown-pragmas -Wfatal-errors -fPIC -Ofast -pthread"
INCLUDES="-I../include -I../src"
LIBS="-lm -pthread"

echo -e "\n${YELLOW}[1/5] Compiling thread synchronization module...${NC}"
gcc -c ../src/thread_sync.c -o thread_sync.o $CFLAGS $INCLUDES

echo -e "${YELLOW}[2/5] Compiling thread safety tests...${NC}"
gcc -c test_thread_safety.c -o test_thread_safety.o $CFLAGS $INCLUDES

echo -e "${YELLOW}[3/5] Linking test executable...${NC}"
gcc test_thread_safety.o thread_sync.o -o test_thread_safety $LIBS

echo -e "${YELLOW}[4/5] Running thread safety tests...${NC}\n"
./test_thread_safety

# Optional: Run with ThreadSanitizer if available
if command -v gcc &> /dev/null && gcc -v 2>&1 | grep -q "gcc version"; then
    echo -e "\n${YELLOW}[5/5] Running with ThreadSanitizer (if available)...${NC}"
    
    # Try to compile with ThreadSanitizer
    if gcc -fsanitize=thread -c ../src/thread_sync.c -o thread_sync_tsan.o $CFLAGS $INCLUDES 2>/dev/null; then
        gcc -fsanitize=thread -c test_thread_safety.c -o test_thread_safety_tsan.o $CFLAGS $INCLUDES
        gcc -fsanitize=thread test_thread_safety_tsan.o thread_sync_tsan.o -o test_thread_safety_tsan $LIBS
        
        echo -e "${GREEN}Running with ThreadSanitizer...${NC}"
        TSAN_OPTIONS="halt_on_error=0 history_size=7" ./test_thread_safety_tsan 2>&1 | tee tsan_output.log
        
        if grep -q "WARNING: ThreadSanitizer" tsan_output.log; then
            echo -e "${RED}ThreadSanitizer detected potential issues!${NC}"
            echo "See tsan_output.log for details"
        else
            echo -e "${GREEN}No issues detected by ThreadSanitizer${NC}"
        fi
        
        # Cleanup TSan files
        rm -f test_thread_safety_tsan test_thread_safety_tsan.o thread_sync_tsan.o
    else
        echo -e "${YELLOW}ThreadSanitizer not available, skipping...${NC}"
    fi
else
    echo -e "${YELLOW}[5/5] ThreadSanitizer check skipped${NC}"
fi

# Cleanup
rm -f test_thread_safety.o thread_sync.o test_thread_safety

echo -e "\n${GREEN}========================================"
echo -e "   All Thread Safety Tests Completed!   "
echo -e "========================================${NC}\n"