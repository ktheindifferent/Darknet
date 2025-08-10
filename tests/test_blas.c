#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "../src/blas.h"

void test_fill_cpu() {
    printf("Testing fill_cpu...\n");
    
    int n = 100;
    float *x = calloc(n, sizeof(float));
    
    fill_cpu(n, 3.14, x, 1);
    
    for(int i = 0; i < n; i++) {
        assert(fabs(x[i] - 3.14) < 0.001);
    }
    
    free(x);
    
    printf("  ✓ fill_cpu tests passed\n");
}

void test_copy_cpu() {
    printf("Testing copy_cpu...\n");
    
    int n = 100;
    float *x = calloc(n, sizeof(float));
    float *y = calloc(n, sizeof(float));
    
    for(int i = 0; i < n; i++) {
        x[i] = i * 0.1;
    }
    
    copy_cpu(n, x, 1, y, 1);
    
    for(int i = 0; i < n; i++) {
        assert(fabs(y[i] - x[i]) < 0.001);
    }
    
    free(x);
    free(y);
    
    printf("  ✓ copy_cpu tests passed\n");
}

void test_axpy_cpu() {
    printf("Testing axpy_cpu...\n");
    
    int n = 100;
    float *x = calloc(n, sizeof(float));
    float *y = calloc(n, sizeof(float));
    
    for(int i = 0; i < n; i++) {
        x[i] = i * 0.1;
        y[i] = i * 0.2;
    }
    
    axpy_cpu(n, 2.0, x, 1, y, 1);
    
    for(int i = 0; i < n; i++) {
        float expected = i * 0.2 + 2.0 * i * 0.1;
        assert(fabs(y[i] - expected) < 0.001);
    }
    
    free(x);
    free(y);
    
    printf("  ✓ axpy_cpu tests passed\n");
}

void test_scal_cpu() {
    printf("Testing scal_cpu...\n");
    
    int n = 100;
    float *x = calloc(n, sizeof(float));
    
    for(int i = 0; i < n; i++) {
        x[i] = i * 0.1;
    }
    
    scal_cpu(n, 2.5, x, 1);
    
    for(int i = 0; i < n; i++) {
        float expected = i * 0.1 * 2.5;
        assert(fabs(x[i] - expected) < 0.001);
    }
    
    free(x);
    
    printf("  ✓ scal_cpu tests passed\n");
}

void test_dot_cpu() {
    printf("Testing dot_cpu...\n");
    
    int n = 100;
    float *x = calloc(n, sizeof(float));
    float *y = calloc(n, sizeof(float));
    
    for(int i = 0; i < n; i++) {
        x[i] = i * 0.1;
        y[i] = i * 0.2;
    }
    
    float dot = dot_cpu(n, x, 1, y, 1);
    
    float expected = 0;
    for(int i = 0; i < n; i++) {
        expected += x[i] * y[i];
    }
    
    assert(fabs(dot - expected) < 0.001);
    
    free(x);
    free(y);
    
    printf("  ✓ dot_cpu tests passed\n");
}

void test_mean_cpu() {
    printf("Testing mean_cpu...\n");
    
    float x[] = {1.0, 2.0, 3.0, 4.0, 5.0};
    
    float mean = mean_cpu(x, 5);
    assert(fabs(mean - 3.0) < 0.001);
    
    float x2[] = {-1.0, -2.0, 0.0, 1.0, 2.0};
    mean = mean_cpu(x2, 5);
    assert(fabs(mean - 0.0) < 0.001);
    
    printf("  ✓ mean_cpu tests passed\n");
}

void test_variance_cpu() {
    printf("Testing variance_cpu...\n");
    
    float x[] = {1.0, 2.0, 3.0, 4.0, 5.0};
    float mean = mean_cpu(x, 5);
    
    float var = variance_cpu(x, 5, mean);
    float expected = 2.0;
    assert(fabs(var - expected) < 0.1);
    
    printf("  ✓ variance_cpu tests passed\n");
}

void test_normalize_cpu() {
    printf("Testing normalize_cpu...\n");
    
    float x[] = {1.0, 2.0, 3.0, 4.0, 5.0};
    float mean[] = {3.0};
    float variance[] = {2.0};
    
    normalize_cpu(x, mean, variance, 1, 5, 5);
    
    for(int i = 0; i < 5; i++) {
        float expected = (i + 1 - 3.0) / sqrt(2.0);
        assert(fabs(x[i] - expected) < 0.01);
    }
    
    printf("  ✓ normalize_cpu tests passed\n");
}

void test_softmax() {
    printf("Testing softmax...\n");
    
    float input[] = {1.0, 2.0, 3.0, 4.0, 5.0};
    int n = 5;
    
    softmax(input, n, 1.0, 1, input, 1);
    
    float sum = 0;
    for(int i = 0; i < n; i++) {
        sum += input[i];
        assert(input[i] > 0);
        assert(input[i] < 1);
    }
    assert(fabs(sum - 1.0) < 0.001);
    
    assert(input[4] > input[3]);
    assert(input[3] > input[2]);
    assert(input[2] > input[1]);
    assert(input[1] > input[0]);
    
    printf("  ✓ softmax tests passed\n");
}

void test_add_bias() {
    printf("Testing add_bias...\n");
    
    float output[] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    float biases[] = {0.5, 1.0, 1.5};
    
    add_bias(output, biases, 1, 3, 2);
    
    assert(fabs(output[0] - 1.5) < 0.001);
    assert(fabs(output[1] - 2.5) < 0.001);
    assert(fabs(output[2] - 4.0) < 0.001);
    assert(fabs(output[3] - 5.0) < 0.001);
    assert(fabs(output[4] - 6.5) < 0.001);
    assert(fabs(output[5] - 7.5) < 0.001);
    
    printf("  ✓ add_bias tests passed\n");
}

void test_scale_bias() {
    printf("Testing scale_bias...\n");
    
    float output[] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    float scales[] = {2.0, 0.5, 1.5};
    
    scale_bias(output, scales, 1, 3, 2);
    
    assert(fabs(output[0] - 2.0) < 0.001);
    assert(fabs(output[1] - 4.0) < 0.001);
    assert(fabs(output[2] - 1.5) < 0.001);
    assert(fabs(output[3] - 2.0) < 0.001);
    assert(fabs(output[4] - 7.5) < 0.001);
    assert(fabs(output[5] - 9.0) < 0.001);
    
    printf("  ✓ scale_bias tests passed\n");
}

void test_l2_cpu() {
    printf("Testing l2_cpu...\n");
    
    float x[] = {3.0, 4.0};
    float l2 = l2_cpu(2, x, 1);
    
    assert(fabs(l2 - 5.0) < 0.001);
    
    float x2[] = {1.0, 0.0, 0.0, 0.0};
    l2 = l2_cpu(4, x2, 1);
    assert(fabs(l2 - 1.0) < 0.001);
    
    printf("  ✓ l2_cpu tests passed\n");
}

int main() {
    printf("\n=== Running BLAS Tests ===\n\n");
    
    test_fill_cpu();
    test_copy_cpu();
    test_axpy_cpu();
    test_scal_cpu();
    test_dot_cpu();
    test_mean_cpu();
    test_variance_cpu();
    test_normalize_cpu();
    test_softmax();
    test_add_bias();
    test_scale_bias();
    test_l2_cpu();
    
    printf("\n=== All BLAS Tests Passed! ===\n\n");
    
    return 0;
}