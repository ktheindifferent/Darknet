#ifndef SAFE_MATH_H
#define SAFE_MATH_H

#include <math.h>
#include <stdio.h>

// Small epsilon value to prevent division by zero
#define SAFE_DIV_EPSILON 1e-10

// Safe division functions
static inline float safe_divide(float numerator, float denominator, float default_value) {
    // Check for NaN or infinity in denominator
    if (!isfinite(denominator) || fabs(denominator) < SAFE_DIV_EPSILON) {
        #ifdef DEBUG
        fprintf(stderr, "Warning: Invalid division prevented. Numerator: %f, Denominator: %f\n", 
                numerator, denominator);
        #endif
        return default_value;
    }
    // Check for NaN in numerator
    if (!isfinite(numerator)) {
        return default_value;
    }
    return numerator / denominator;
}

static inline float safe_divide_eps(float numerator, float denominator) {
    // Handle NaN/Inf cases
    if (!isfinite(numerator)) {
        return 0.0;
    }
    if (!isfinite(denominator)) {
        return 0.0;
    }
    // Add epsilon to denominator to prevent division by zero
    return numerator / (denominator + SAFE_DIV_EPSILON);
}

static inline float safe_sqrt_divide(float numerator, float denominator, float default_value) {
    // Check for NaN/Inf
    if (!isfinite(numerator) || !isfinite(denominator)) {
        return default_value;
    }
    if (denominator <= 0 || numerator < 0) {
        #ifdef DEBUG
        fprintf(stderr, "Warning: Invalid sqrt division. Numerator: %f, Denominator: %f\n", 
                numerator, denominator);
        #endif
        return default_value;
    }
    return sqrt(numerator / denominator);
}

static inline float safe_log(float value, float default_value) {
    // Check for NaN/Inf
    if (!isfinite(value)) {
        return default_value;
    }
    if (value <= 0) {
        #ifdef DEBUG
        fprintf(stderr, "Warning: Log of non-positive value prevented. Value: %f\n", value);
        #endif
        return default_value;
    }
    return log(value);
}

// Safe integer division for random operations
static inline size_t safe_rand_divide(size_t range) {
    if (range == 0) {
        return 0;
    }
    return rand() / (RAND_MAX / range + 1);
}

#endif // SAFE_MATH_H