#!/bin/bash

# Performance Testing Script for Thread Synchronization

set -e

echo "========================================"
echo "  Thread Synchronization Performance Test"
echo "========================================"

# Compilation flags
CFLAGS="-Wall -Wno-unused-result -Wno-unknown-pragmas -Wfatal-errors -fPIC -O3 -pthread -march=native"
INCLUDES="-I../include -I../src"
LIBS="-lm -pthread"

echo -e "\nCompiling performance tests..."
gcc -c ../src/thread_sync.c -o thread_sync.o $CFLAGS $INCLUDES
gcc -c test_thread_performance.c -o test_thread_performance.o $CFLAGS $INCLUDES
gcc test_thread_performance.o thread_sync.o -o test_thread_performance $LIBS

echo -e "\nRunning performance tests...\n"
./test_thread_performance

# Cleanup
rm -f test_thread_performance.o thread_sync.o test_thread_performance

echo -e "\nPerformance testing complete!"