#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <float.h>
#include <stdlib.h>
#include "../src/safe_math.h"

#define TOLERANCE 1e-6

void test_safe_divide() {
    printf("Testing safe_divide...\n");
    
    // Normal division
    assert(fabs(safe_divide(10.0, 2.0, 0.0) - 5.0) < TOLERANCE);
    assert(fabs(safe_divide(-10.0, 2.0, 0.0) - (-5.0)) < TOLERANCE);
    
    // Division by zero - should return default value
    assert(fabs(safe_divide(10.0, 0.0, -1.0) - (-1.0)) < TOLERANCE);
    assert(fabs(safe_divide(10.0, 0.0, 999.0) - 999.0) < TOLERANCE);
    
    // Division by very small number (less than epsilon)
    assert(fabs(safe_divide(10.0, 1e-11, 42.0) - 42.0) < TOLERANCE);
    assert(fabs(safe_divide(10.0, -1e-11, 42.0) - 42.0) < TOLERANCE);
    
    // Edge cases
    assert(fabs(safe_divide(0.0, 5.0, 1.0) - 0.0) < TOLERANCE);
    assert(fabs(safe_divide(0.0, 0.0, 7.0) - 7.0) < TOLERANCE);
    
    printf("  ✓ safe_divide tests passed\n");
}

void test_safe_divide_eps() {
    printf("Testing safe_divide_eps...\n");
    
    // Normal division (with epsilon added)
    float result = safe_divide_eps(10.0, 2.0);
    assert(result > 0 && result <= 5.0); // Should be approximately 5 (slightly less due to epsilon)
    
    // Division by zero - epsilon prevents actual division by zero
    result = safe_divide_eps(10.0, 0.0);
    assert(isfinite(result));
    assert(result > 0);
    
    // Division by negative number
    result = safe_divide_eps(10.0, -2.0);
    assert(result < 0);
    
    // Small denominator
    result = safe_divide_eps(1.0, 1e-12);
    assert(isfinite(result));
    
    printf("  ✓ safe_divide_eps tests passed\n");
}

void test_safe_sqrt_divide() {
    printf("Testing safe_sqrt_divide...\n");
    
    // Normal sqrt division
    assert(fabs(safe_sqrt_divide(25.0, 1.0, 0.0) - 5.0) < TOLERANCE);
    assert(fabs(safe_sqrt_divide(100.0, 4.0, 0.0) - 5.0) < TOLERANCE);
    
    // Division by zero or negative denominator
    assert(fabs(safe_sqrt_divide(25.0, 0.0, -1.0) - (-1.0)) < TOLERANCE);
    assert(fabs(safe_sqrt_divide(25.0, -1.0, -2.0) - (-2.0)) < TOLERANCE);
    
    // Negative numerator
    assert(fabs(safe_sqrt_divide(-25.0, 1.0, 99.0) - 99.0) < TOLERANCE);
    
    // Both negative (still invalid for sqrt)
    assert(fabs(safe_sqrt_divide(-25.0, -1.0, 42.0) - 42.0) < TOLERANCE);
    
    // Zero numerator
    assert(fabs(safe_sqrt_divide(0.0, 1.0, 1.0) - 0.0) < TOLERANCE);
    
    printf("  ✓ safe_sqrt_divide tests passed\n");
}

void test_safe_log() {
    printf("Testing safe_log...\n");
    
    // Normal log
    assert(fabs(safe_log(M_E, 0.0) - 1.0) < TOLERANCE);
    assert(fabs(safe_log(1.0, 0.0) - 0.0) < TOLERANCE);
    
    // Log of zero - should return default
    assert(fabs(safe_log(0.0, -999.0) - (-999.0)) < TOLERANCE);
    
    // Log of negative - should return default
    assert(fabs(safe_log(-1.0, 123.0) - 123.0) < TOLERANCE);
    assert(fabs(safe_log(-100.0, 456.0) - 456.0) < TOLERANCE);
    
    // Very small positive number
    float result = safe_log(1e-10, 0.0);
    assert(isfinite(result));
    assert(result < 0); // log of small positive number is negative
    
    printf("  ✓ safe_log tests passed\n");
}

void test_safe_rand_divide() {
    printf("Testing safe_rand_divide...\n");
    
    // Division by zero - should return 0
    assert(safe_rand_divide(0) == 0);
    
    // Normal cases - result should be in range [0, range)
    for (int i = 0; i < 100; i++) {
        size_t result = safe_rand_divide(10);
        assert(result >= 0 && result < 10);
    }
    
    // Range of 1 - should always return 0
    for (int i = 0; i < 10; i++) {
        assert(safe_rand_divide(1) == 0);
    }
    
    printf("  ✓ safe_rand_divide tests passed\n");
}

void test_edge_cases() {
    printf("Testing edge cases...\n");
    
    // Infinity handling
    float inf = INFINITY;
    
    // Division with infinity
    assert(isinf(safe_divide(inf, 2.0, 0.0)));
    assert(fabs(safe_divide(10.0, inf, 42.0) - 0.0) < TOLERANCE);
    
    // NaN handling
    float nan_val = NAN;
    float result = safe_divide(nan_val, 2.0, 0.0);
    assert(isnan(result));
    
    // Very large numbers
    float large = 1e38;
    result = safe_divide(large, 2.0, 0.0);
    assert(isfinite(result));
    assert(fabs(result - large/2.0) < large * TOLERANCE);
    
    // Very small positive numbers
    float tiny = 1e-38;
    result = safe_divide(1.0, tiny, 999.0);
    assert(isfinite(result) || fabs(result - 999.0) < TOLERANCE);
    
    printf("  ✓ Edge case tests passed\n");
}

int main() {
    printf("\n=== Running Safe Math Division Tests ===\n\n");
    
    test_safe_divide();
    test_safe_divide_eps();
    test_safe_sqrt_divide();
    test_safe_log();
    test_safe_rand_divide();
    test_edge_cases();
    
    printf("\n=== All Safe Math Tests Passed! ===\n\n");
    return 0;
}