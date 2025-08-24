#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "normalization_layer.h"
#include "network.h"
#include "utils.h"
#include "blas.h"

void print_array(const char* name, float* arr, int n) {
    printf("%s: ", name);
    for(int i = 0; i < n; i++) {
        printf("%.4f ", arr[i]);
    }
    printf("\n");
}

int main() {
    // Simple test with small dimensions
    int batch = 1;
    int w = 2, h = 2, c = 3;
    int size = 3;
    float alpha = 1.0f, beta = 0.5f, kappa = 1.0f;
    
    printf("Testing normalization layer with:\n");
    printf("batch=%d, w=%d, h=%d, c=%d, size=%d\n", batch, w, h, c, size);
    printf("alpha=%.2f, beta=%.2f, kappa=%.2f\n\n", alpha, beta, kappa);
    
    layer l = make_normalization_layer(batch, w, h, c, size, alpha, beta, kappa);
    
    network net = {0};
    net.batch = batch;
    int total_size = w * h * c * batch;
    
    // Create simple input data
    net.input = calloc(total_size, sizeof(float));
    for(int i = 0; i < total_size; i++) {
        net.input[i] = (i + 1) * 0.1f;
    }
    
    printf("Input data:\n");
    print_array("input", net.input, total_size);
    
    // Forward pass
    forward_normalization_layer(l, net);
    
    printf("\nAfter forward pass:\n");
    print_array("squared", l.squared, total_size);
    print_array("norms", l.norms, total_size);
    print_array("output", l.output, total_size);
    
    // Set gradient for backward pass
    for(int i = 0; i < total_size; i++) {
        l.delta[i] = 1.0f; // Simple gradient
    }
    
    net.delta = calloc(total_size, sizeof(float));
    
    // Backward pass
    backward_normalization_layer(l, net);
    
    printf("\nAfter backward pass:\n");
    print_array("delta_in", net.delta, total_size);
    
    // Test numerical gradient for one element
    printf("\nNumerical gradient check for index 0:\n");
    float epsilon = 1e-5;
    float original = net.input[0];
    
    // Forward with input + epsilon
    net.input[0] = original + epsilon;
    forward_normalization_layer(l, net);
    float loss_plus = 0;
    for(int i = 0; i < total_size; i++) {
        loss_plus += l.output[i];
    }
    
    // Forward with input - epsilon
    net.input[0] = original - epsilon;
    forward_normalization_layer(l, net);
    float loss_minus = 0;
    for(int i = 0; i < total_size; i++) {
        loss_minus += l.output[i];
    }
    
    float numerical_grad = (loss_plus - loss_minus) / (2 * epsilon);
    printf("Analytical gradient: %.6f\n", net.delta[0]);
    printf("Numerical gradient: %.6f\n", numerical_grad);
    printf("Difference: %.6f\n", fabs(net.delta[0] - numerical_grad));
    
    // Cleanup
    free(net.input);
    free(net.delta);
    free(l.output);
    free(l.delta);
    free(l.squared);
    free(l.norms);
    
    return 0;
}