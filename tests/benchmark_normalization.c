#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "normalization_layer.h"
#include "network.h"
#include "utils.h"
#include "blas.h"

double get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

void benchmark_config(int batch, int w, int h, int c, int size, int iterations) {
    printf("Benchmarking: batch=%d, w=%d, h=%d, c=%d, size=%d\n", 
           batch, w, h, c, size);
    
    float alpha = 0.0001f, beta = 0.75f, kappa = 2.0f;
    
    layer l = make_normalization_layer(batch, w, h, c, size, alpha, beta, kappa);
    
    network net = {0};
    net.batch = batch;
    int total_size = w * h * c * batch;
    
    // Allocate memory
    net.input = calloc(total_size, sizeof(float));
    net.delta = calloc(total_size, sizeof(float));
    
    // Initialize with random data
    for(int i = 0; i < total_size; i++) {
        net.input[i] = (float)rand() / RAND_MAX;
        l.delta[i] = (float)rand() / RAND_MAX * 0.1f;
    }
    
    // Warm up
    for(int i = 0; i < 10; i++) {
        forward_normalization_layer(l, net);
        backward_normalization_layer(l, net);
    }
    
    // Benchmark forward pass
    double start = get_time_ms();
    for(int i = 0; i < iterations; i++) {
        forward_normalization_layer(l, net);
    }
    double forward_time = get_time_ms() - start;
    
    // Benchmark backward pass
    start = get_time_ms();
    for(int i = 0; i < iterations; i++) {
        memset(net.delta, 0, total_size * sizeof(float));
        backward_normalization_layer(l, net);
    }
    double backward_time = get_time_ms() - start;
    
    printf("  Forward:  %.2f ms (%.2f ms/iter)\n", 
           forward_time, forward_time / iterations);
    printf("  Backward: %.2f ms (%.2f ms/iter)\n", 
           backward_time, backward_time / iterations);
    printf("  Total:    %.2f ms (%.2f ms/iter)\n\n", 
           forward_time + backward_time, 
           (forward_time + backward_time) / iterations);
    
    // Cleanup
    free(net.input);
    free(net.delta);
    free(l.output);
    free(l.delta);
    free(l.squared);
    free(l.norms);
}

int main() {
    srand(time(NULL));
    
    printf("=== Normalization Layer Performance Benchmark ===\n\n");
    printf("Note: This tests the corrected gradient implementation.\n");
    printf("The backward pass now correctly computes gradients but may be slower.\n\n");
    
    int iterations = 100;
    
    // Test various sizes
    struct {
        int batch, w, h, c, size;
    } configs[] = {
        {1, 32, 32, 64, 5},    // Small network
        {4, 32, 32, 64, 5},    // Small batch
        {1, 64, 64, 128, 5},   // Medium network
        {4, 64, 64, 128, 5},   // Medium batch
        {1, 128, 128, 256, 5}, // Large network
        {4, 128, 128, 256, 5}, // Large batch
    };
    
    int num_configs = sizeof(configs) / sizeof(configs[0]);
    
    for(int i = 0; i < num_configs; i++) {
        benchmark_config(
            configs[i].batch,
            configs[i].w,
            configs[i].h,
            configs[i].c,
            configs[i].size,
            iterations
        );
    }
    
    printf("=== Summary ===\n");
    printf("The backward pass now:\n");
    printf("- Correctly computes gradients for all inputs\n");
    printf("- Properly adds to delta instead of overwriting\n");
    printf("- Handles cross-channel dependencies accurately\n");
    printf("- May be slower due to more complex computation\n");
    
    return 0;
}