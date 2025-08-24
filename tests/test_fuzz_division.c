#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <float.h>
#include "../src/safe_math.h"

// Fuzzing test to ensure division protection handles extreme values
void fuzz_safe_divide(int iterations) {
    printf("Fuzzing safe_divide with %d iterations...\n", iterations);
    
    for (int i = 0; i < iterations; i++) {
        // Generate random values including special cases
        float numerator = (float)(rand() - RAND_MAX/2) / (rand() % 1000 + 1);
        float denominator = (float)(rand() - RAND_MAX/2) / (rand() % 1000000 + 1);
        float default_val = (float)(rand() - RAND_MAX/2) / 1000.0;
        
        // Sometimes inject special values
        if (rand() % 100 < 5) denominator = 0.0;
        if (rand() % 100 < 2) denominator = INFINITY;
        if (rand() % 100 < 2) denominator = -INFINITY;
        if (rand() % 100 < 2) denominator = NAN;
        if (rand() % 100 < 5) denominator = FLT_MIN;
        if (rand() % 100 < 5) denominator = -FLT_MIN;
        
        float result = safe_divide(numerator, denominator, default_val);
        
        // Verify result is always finite or equals default
        if (!isfinite(result) && result != default_val) {
            printf("  FAIL: safe_divide(%f, %f, %f) = %f\n", 
                   numerator, denominator, default_val, result);
            exit(1);
        }
    }
    printf("  ✓ safe_divide fuzzing passed\n");
}

void fuzz_safe_sqrt_divide(int iterations) {
    printf("Fuzzing safe_sqrt_divide with %d iterations...\n", iterations);
    
    for (int i = 0; i < iterations; i++) {
        float numerator = (float)(rand() - RAND_MAX/2) / (rand() % 1000 + 1);
        float denominator = (float)(rand() - RAND_MAX/2) / (rand() % 1000 + 1);
        float default_val = (float)rand() / 1000.0;
        
        // Sometimes inject problematic values
        if (rand() % 100 < 10) numerator = -fabs(numerator);  // Negative
        if (rand() % 100 < 10) denominator = 0.0;
        if (rand() % 100 < 5) denominator = -fabs(denominator);  // Negative
        
        float result = safe_sqrt_divide(numerator, denominator, default_val);
        
        // Verify result is valid
        if (!isfinite(result) && result != default_val) {
            printf("  FAIL: safe_sqrt_divide(%f, %f, %f) = %f\n", 
                   numerator, denominator, default_val, result);
            exit(1);
        }
        
        // sqrt result should never be negative (unless it's the default)
        if (result < 0 && result != default_val) {
            printf("  FAIL: safe_sqrt_divide returned negative: %f\n", result);
            exit(1);
        }
    }
    printf("  ✓ safe_sqrt_divide fuzzing passed\n");
}

void fuzz_safe_log(int iterations) {
    printf("Fuzzing safe_log with %d iterations...\n", iterations);
    
    for (int i = 0; i < iterations; i++) {
        float value = (float)(rand() - RAND_MAX/2) / (rand() % 1000 + 1);
        float default_val = (float)(rand() - RAND_MAX/2) / 1000.0;
        
        // Sometimes inject problematic values
        if (rand() % 100 < 10) value = 0.0;
        if (rand() % 100 < 10) value = -fabs(value);  // Negative
        if (rand() % 100 < 2) value = INFINITY;
        if (rand() % 100 < 2) value = -INFINITY;
        
        float result = safe_log(value, default_val);
        
        // Verify result is valid
        if (!isfinite(result) && result != default_val) {
            printf("  FAIL: safe_log(%f, %f) = %f\n", value, default_val, result);
            exit(1);
        }
    }
    printf("  ✓ safe_log fuzzing passed\n");
}

void fuzz_safe_divide_eps(int iterations) {
    printf("Fuzzing safe_divide_eps with %d iterations...\n", iterations);
    
    for (int i = 0; i < iterations; i++) {
        float numerator = (float)(rand() - RAND_MAX/2) / (rand() % 1000 + 1);
        float denominator = (float)(rand() - RAND_MAX/2) / (rand() % 1000000 + 1);
        
        // Sometimes inject special values
        if (rand() % 100 < 10) denominator = 0.0;
        if (rand() % 100 < 2) denominator = -0.0;
        if (rand() % 100 < 5) denominator = FLT_MIN;
        if (rand() % 100 < 5) denominator = -FLT_MIN;
        
        float result = safe_divide_eps(numerator, denominator);
        
        // Result should always be finite
        if (!isfinite(result)) {
            printf("  FAIL: safe_divide_eps(%f, %f) = %f (not finite)\n", 
                   numerator, denominator, result);
            exit(1);
        }
    }
    printf("  ✓ safe_divide_eps fuzzing passed\n");
}

int main() {
    printf("\n=== Running Division Protection Fuzzing Tests ===\n\n");
    
    // Seed random number generator
    srand(time(NULL));
    
    int iterations = 10000;
    
    fuzz_safe_divide(iterations);
    fuzz_safe_sqrt_divide(iterations);
    fuzz_safe_log(iterations);
    fuzz_safe_divide_eps(iterations);
    
    printf("\n=== All Fuzzing Tests Passed (%d iterations each) ===\n\n", iterations);
    return 0;
}