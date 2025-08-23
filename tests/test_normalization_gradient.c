#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "normalization_layer.h"
#include "network.h"
#include "utils.h"
#include "blas.h"

#define EPSILON 1e-5
#define GRADIENT_CHECK_EPSILON 1e-7
#define RELATIVE_ERROR_THRESHOLD 1e-4

// Helper function to initialize random data
void init_random_data(float *data, int n, float scale) {
    for(int i = 0; i < n; i++) {
        data[i] = scale * (2.0f * rand() / RAND_MAX - 1.0f);
    }
}

// Compute numerical gradient using finite differences
float compute_numerical_gradient(layer l, network net, float *input, int idx, float *output_grad) {
    int total_size = l.inputs * l.batch;
    float original = input[idx];
    
    // Forward pass with input + epsilon
    input[idx] = original + GRADIENT_CHECK_EPSILON;
    forward_normalization_layer(l, net);
    
    float loss_plus = 0;
    for(int i = 0; i < total_size; i++) {
        loss_plus += l.output[i] * output_grad[i];
    }
    
    // Forward pass with input - epsilon
    input[idx] = original - GRADIENT_CHECK_EPSILON;
    forward_normalization_layer(l, net);
    
    float loss_minus = 0;
    for(int i = 0; i < total_size; i++) {
        loss_minus += l.output[i] * output_grad[i];
    }
    
    // Restore original value
    input[idx] = original;
    
    // Compute numerical gradient
    return (loss_plus - loss_minus) / (2.0f * GRADIENT_CHECK_EPSILON);
}

// Test gradient computation for a single layer
int test_normalization_gradient(int batch, int w, int h, int c, int size, float alpha, float beta, float kappa) {
    printf("Testing gradient for batch=%d, w=%d, h=%d, c=%d, size=%d, alpha=%.3f, beta=%.3f, kappa=%.3f\n",
           batch, w, h, c, size, alpha, beta, kappa);
    
    // Create layer
    layer l = make_normalization_layer(batch, w, h, c, size, alpha, beta, kappa);
    
    // Create network context
    network net = {0};
    net.batch = batch;
    int total_size = w * h * c * batch;
    
    // Allocate and initialize input
    net.input = calloc(total_size, sizeof(float));
    init_random_data(net.input, total_size, 1.0f);
    
    // Allocate delta for backward pass
    net.delta = calloc(total_size, sizeof(float));
    
    // Initialize output gradient
    float *output_grad = calloc(total_size, sizeof(float));
    init_random_data(output_grad, total_size, 0.1f);
    
    // Forward pass
    forward_normalization_layer(l, net);
    
    // Copy output gradient to layer delta
    memcpy(l.delta, output_grad, total_size * sizeof(float));
    
    // Backward pass
    memset(net.delta, 0, total_size * sizeof(float));
    backward_normalization_layer(l, net);
    
    // Check gradients at random positions
    int num_checks = 100;
    int errors = 0;
    float max_error = 0;
    float avg_error = 0;
    
    for(int check = 0; check < num_checks; check++) {
        int idx = rand() % total_size;
        
        // Get analytical gradient from backward pass
        float analytical_grad = net.delta[idx];
        
        // Compute numerical gradient
        float numerical_grad = compute_numerical_gradient(l, net, net.input, idx, output_grad);
        
        // Compute relative error
        float error = fabs(analytical_grad - numerical_grad);
        float denom = fmax(fabs(analytical_grad), fabs(numerical_grad)) + 1e-8f;
        float relative_error = error / denom;
        
        avg_error += relative_error;
        max_error = fmax(max_error, relative_error);
        
        if(relative_error > RELATIVE_ERROR_THRESHOLD) {
            if(errors < 5) { // Print first 5 errors
                printf("  ERROR at idx %d: analytical=%.8f, numerical=%.8f, rel_error=%.6f\n",
                       idx, analytical_grad, numerical_grad, relative_error);
            }
            errors++;
        }
    }
    
    avg_error /= num_checks;
    
    printf("  Gradient check: %d/%d passed, max_error=%.6f, avg_error=%.6f\n",
           num_checks - errors, num_checks, max_error, avg_error);
    
    // Cleanup
    free(net.input);
    free(net.delta);
    free(output_grad);
    free(l.output);
    free(l.delta);
    free(l.squared);
    free(l.norms);
    
    return errors == 0;
}

// Test that gradient properly accumulates (adds to delta instead of overwriting)
int test_gradient_accumulation() {
    printf("Testing gradient accumulation...\n");
    
    int batch = 2;
    int w = 4, h = 4, c = 8;
    int size = 5;
    float alpha = 0.0001f, beta = 0.75f, kappa = 2.0f;
    
    layer l = make_normalization_layer(batch, w, h, c, size, alpha, beta, kappa);
    
    network net = {0};
    net.batch = batch;
    int total_size = w * h * c * batch;
    
    net.input = calloc(total_size, sizeof(float));
    net.delta = calloc(total_size, sizeof(float));
    
    // Initialize with random data
    init_random_data(net.input, total_size, 1.0f);
    init_random_data(l.delta, total_size, 0.1f);
    
    // Set initial delta to non-zero values
    for(int i = 0; i < total_size; i++) {
        net.delta[i] = 0.5f;
    }
    
    // Forward pass
    forward_normalization_layer(l, net);
    
    // Save initial delta
    float *initial_delta = calloc(total_size, sizeof(float));
    memcpy(initial_delta, net.delta, total_size * sizeof(float));
    
    // Backward pass
    backward_normalization_layer(l, net);
    
    // Check that delta has changed (should have added to it)
    int accumulation_ok = 1;
    for(int i = 0; i < total_size; i++) {
        if(net.delta[i] == initial_delta[i]) {
            printf("  ERROR: Delta not updated at index %d\n", i);
            accumulation_ok = 0;
            break;
        }
    }
    
    if(accumulation_ok) {
        printf("  Gradient accumulation: PASSED\n");
    } else {
        printf("  Gradient accumulation: FAILED\n");
    }
    
    // Cleanup
    free(net.input);
    free(net.delta);
    free(initial_delta);
    free(l.output);
    free(l.delta);
    free(l.squared);
    free(l.norms);
    
    return accumulation_ok;
}

// Compare CPU and GPU implementations
#ifdef GPU
int test_cpu_gpu_consistency() {
    printf("Testing CPU/GPU consistency...\n");
    
    int batch = 2;
    int w = 8, h = 8, c = 16;
    int size = 5;
    float alpha = 0.0001f, beta = 0.75f, kappa = 2.0f;
    
    layer l = make_normalization_layer(batch, w, h, c, size, alpha, beta, kappa);
    
    network net = {0};
    net.batch = batch;
    int total_size = w * h * c * batch;
    
    // CPU data
    net.input = calloc(total_size, sizeof(float));
    net.delta = calloc(total_size, sizeof(float));
    init_random_data(net.input, total_size, 1.0f);
    init_random_data(l.delta, total_size, 0.1f);
    
    // Save CPU results
    float *cpu_delta = calloc(total_size, sizeof(float));
    
    // CPU forward and backward
    forward_normalization_layer(l, net);
    backward_normalization_layer(l, net);
    memcpy(cpu_delta, net.delta, total_size * sizeof(float));
    
    // GPU forward and backward
    cuda_push_array(net.input_gpu, net.input, total_size);
    cuda_push_array(l.delta_gpu, l.delta, total_size);
    memset(net.delta, 0, total_size * sizeof(float));
    cuda_push_array(net.delta_gpu, net.delta, total_size);
    
    forward_normalization_layer_gpu(l, net);
    backward_normalization_layer_gpu(l, net);
    
    float *gpu_delta = calloc(total_size, sizeof(float));
    cuda_pull_array(net.delta_gpu, gpu_delta, total_size);
    
    // Compare results
    float max_diff = 0;
    for(int i = 0; i < total_size; i++) {
        float diff = fabs(cpu_delta[i] - gpu_delta[i]);
        max_diff = fmax(max_diff, diff);
    }
    
    printf("  Max difference between CPU and GPU: %.8f\n", max_diff);
    
    int consistent = (max_diff < 1e-5);
    if(consistent) {
        printf("  CPU/GPU consistency: PASSED\n");
    } else {
        printf("  CPU/GPU consistency: FAILED\n");
    }
    
    // Cleanup
    free(net.input);
    free(net.delta);
    free(cpu_delta);
    free(gpu_delta);
    free(l.output);
    free(l.delta);
    free(l.squared);
    free(l.norms);
    cuda_free(l.output_gpu);
    cuda_free(l.delta_gpu);
    cuda_free(l.squared_gpu);
    cuda_free(l.norms_gpu);
    
    return consistent;
}
#endif

int main() {
    srand(time(NULL));
    
    printf("=== Normalization Layer Gradient Tests ===\n\n");
    
    int all_passed = 1;
    
    // Test gradient accumulation
    if(!test_gradient_accumulation()) {
        all_passed = 0;
    }
    printf("\n");
    
    // Test various configurations
    struct {
        int batch, w, h, c, size;
        float alpha, beta, kappa;
    } test_configs[] = {
        {1, 4, 4, 8, 5, 0.0001f, 0.75f, 2.0f},    // Small test
        {2, 8, 8, 16, 5, 0.0001f, 0.75f, 2.0f},   // Medium test
        {1, 16, 16, 32, 5, 0.0001f, 0.75f, 2.0f}, // Larger test
        {2, 4, 4, 8, 3, 0.001f, 0.5f, 1.0f},      // Different parameters
        {1, 8, 8, 4, 7, 0.00005f, 1.0f, 2.0f},    // Different size
    };
    
    int num_configs = sizeof(test_configs) / sizeof(test_configs[0]);
    
    for(int i = 0; i < num_configs; i++) {
        if(!test_normalization_gradient(
            test_configs[i].batch,
            test_configs[i].w,
            test_configs[i].h,
            test_configs[i].c,
            test_configs[i].size,
            test_configs[i].alpha,
            test_configs[i].beta,
            test_configs[i].kappa)) {
            all_passed = 0;
        }
        printf("\n");
    }
    
    #ifdef GPU
    if(!test_cpu_gpu_consistency()) {
        all_passed = 0;
    }
    printf("\n");
    #endif
    
    printf("=== Summary ===\n");
    if(all_passed) {
        printf("All gradient tests PASSED!\n");
        return 0;
    } else {
        printf("Some gradient tests FAILED!\n");
        return 1;
    }
}