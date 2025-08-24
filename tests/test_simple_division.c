#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include "../src/safe_math.h"

#define TOLERANCE 1e-6

// Simple tests for the math functions we fixed
void test_division_scenarios() {
    printf("Testing division by zero scenarios...\n");
    
    // Test safe_divide
    float result = safe_divide(10.0, 0.0, -1.0);
    assert(fabs(result - (-1.0)) < TOLERANCE);
    printf("  ✓ safe_divide(10.0, 0.0, -1.0) = %f (expected -1.0)\n", result);
    
    // Test safe_divide_eps
    result = safe_divide_eps(10.0, 0.0);
    assert(isfinite(result));
    printf("  ✓ safe_divide_eps(10.0, 0.0) = %f (finite result)\n", result);
    
    // Test safe_sqrt_divide
    result = safe_sqrt_divide(25.0, 0.0, 999.0);
    assert(fabs(result - 999.0) < TOLERANCE);
    printf("  ✓ safe_sqrt_divide(25.0, 0.0, 999.0) = %f (expected 999.0)\n", result);
    
    // Test safe_log
    result = safe_log(0.0, -5.0);
    assert(fabs(result - (-5.0)) < TOLERANCE);
    printf("  ✓ safe_log(0.0, -5.0) = %f (expected -5.0)\n", result);
    
    result = safe_log(-10.0, 100.0);
    assert(fabs(result - 100.0) < TOLERANCE);
    printf("  ✓ safe_log(-10.0, 100.0) = %f (expected 100.0)\n", result);
    
    printf("\nAll division protection tests passed!\n");
}

void test_edge_cases() {
    printf("\nTesting edge cases...\n");
    
    // Test with very small denominators
    float result = safe_divide(1.0, 1e-15, 0.0);
    printf("  ✓ safe_divide(1.0, 1e-15, 0.0) = %f (should be 0.0)\n", result);
    assert(fabs(result - 0.0) < TOLERANCE);
    
    // Test with negative very small denominators
    result = safe_divide(1.0, -1e-15, 42.0);
    printf("  ✓ safe_divide(1.0, -1e-15, 42.0) = %f (should be 42.0)\n", result);
    assert(fabs(result - 42.0) < TOLERANCE);
    
    // Test sqrt with negative numerator
    result = safe_sqrt_divide(-100.0, 4.0, 99.0);
    printf("  ✓ safe_sqrt_divide(-100.0, 4.0, 99.0) = %f (should be 99.0)\n", result);
    assert(fabs(result - 99.0) < TOLERANCE);
    
    // Test sqrt with zero denominator
    result = safe_sqrt_divide(100.0, 0.0, 77.0);
    printf("  ✓ safe_sqrt_divide(100.0, 0.0, 77.0) = %f (should be 77.0)\n", result);
    assert(fabs(result - 77.0) < TOLERANCE);
    
    printf("\nAll edge case tests passed!\n");
}

void test_normal_operations() {
    printf("\nTesting normal operations (sanity check)...\n");
    
    // Normal division should work correctly
    float result = safe_divide(10.0, 2.0, 0.0);
    assert(fabs(result - 5.0) < TOLERANCE);
    printf("  ✓ safe_divide(10.0, 2.0, 0.0) = %f (expected 5.0)\n", result);
    
    // Normal sqrt division
    result = safe_sqrt_divide(100.0, 4.0, 0.0);
    assert(fabs(result - 5.0) < TOLERANCE);
    printf("  ✓ safe_sqrt_divide(100.0, 4.0, 0.0) = %f (expected 5.0)\n", result);
    
    // Normal log
    result = safe_log(M_E, 0.0);
    assert(fabs(result - 1.0) < TOLERANCE);
    printf("  ✓ safe_log(e, 0.0) = %f (expected 1.0)\n", result);
    
    printf("\nAll normal operation tests passed!\n");
}

int main() {
    printf("\n=== Running Simple Division Protection Tests ===\n\n");
    
    test_division_scenarios();
    test_edge_cases();
    test_normal_operations();
    
    printf("\n=== All Simple Division Tests Passed! ===\n\n");
    return 0;
}