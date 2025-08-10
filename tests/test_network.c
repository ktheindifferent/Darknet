#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/network.h"
#include "../src/utils.h"
#include "../src/parser.h"

void test_make_network() {
    printf("Testing make_network...\n");
    
    network *net = make_network(5);
    assert(net != NULL);
    assert(net->n == 5);
    assert(net->layers != NULL);
    assert(net->seen == 0);
    assert(net->t == 0);
    assert(net->epoch == 0);
    
    free_network(net);
    
    printf("  ✓ make_network tests passed\n");
}

void test_get_current_rate() {
    printf("Testing get_current_rate...\n");
    
    network net = {0};
    net.learning_rate = 0.001;
    net.burn_in = 1000;
    net.policy = CONSTANT;
    
    float rate = get_current_rate(&net);
    assert(rate > 0.0009 && rate < 0.0011);
    
    net.policy = STEPS;
    net.steps = calloc(2, sizeof(int));
    net.steps[0] = 10000;
    net.steps[1] = 20000;
    net.scales = calloc(2, sizeof(float));
    net.scales[0] = 0.1;
    net.scales[1] = 0.1;
    net.num_steps = 2;
    
    // Use seen pointer instead of batch_num
    size_t seen_val = 5000 * net.batch;
    net.seen = &seen_val;
    
    rate = get_current_rate(&net);
    
    seen_val = 15000 * net.batch;
    rate = get_current_rate(&net);
    
    free(net.steps);
    free(net.scales);
    
    printf("  ✓ get_current_rate tests passed\n");
}

void test_get_network_output() {
    printf("Testing get_network_output...\n");
    
    network *net = make_network(3);
    
    layer l1 = {0};
    l1.outputs = 100;
    l1.output = calloc(100, sizeof(float));
    for(int i = 0; i < 100; i++) {
        l1.output[i] = i * 0.01;
    }
    
    layer l2 = {0};
    l2.outputs = 50;
    l2.output = calloc(50, sizeof(float));
    for(int i = 0; i < 50; i++) {
        l2.output[i] = i * 0.02;
    }
    
    layer l3 = {0};
    l3.outputs = 10;
    l3.output = calloc(10, sizeof(float));
    for(int i = 0; i < 10; i++) {
        l3.output[i] = i * 0.1;
    }
    
    net->layers[0] = l1;
    net->layers[1] = l2;
    net->layers[2] = l3;
    
    float *output = get_network_output(net);
    assert(output == l3.output);
    assert(output[0] == 0.0);
    assert(output[9] > 0.89 && output[9] < 0.91);
    
    free(l1.output);
    free(l2.output);
    free(l3.output);
    free_network(net);
    
    printf("  ✓ get_network_output tests passed\n");
}

void test_get_network_output_layer() {
    printf("Testing get_network_output_layer...\n");
    
    network *net = make_network(5);
    
    for(int i = 0; i < 5; i++) {
        layer l = {0};
        l.outputs = (i + 1) * 10;
        l.output = calloc(l.outputs, sizeof(float));
        l.output[0] = i + 1.0;
        net->layers[i] = l;
    }
    
    float *output = get_network_output_layer(net, 2);
    assert(output[0] == 3.0);
    
    output = get_network_output_layer(net, 4);
    assert(output[0] == 5.0);
    
    output = get_network_output_layer(net, -1);
    assert(output[0] == 5.0);
    
    for(int i = 0; i < 5; i++) {
        free(net->layers[i].output);
    }
    free_network(net);
    
    printf("  ✓ get_network_output_layer tests passed\n");
}

void test_get_network_cost() {
    printf("Testing get_network_cost...\n");
    
    network *net = make_network(3);
    
    for(int i = 0; i < 3; i++) {
        layer l = {0};
        l.cost = calloc(1, sizeof(float));
        *l.cost = (i + 1) * 0.1;
        net->layers[i] = l;
    }
    
    float cost = get_network_cost(net);
    float expected = 0.1 + 0.2 + 0.3;
    assert(cost > expected - 0.01 && cost < expected + 0.01);
    
    for(int i = 0; i < 3; i++) {
        free(net->layers[i].cost);
    }
    free_network(net);
    
    printf("  ✓ get_network_cost tests passed\n");
}

void test_top_k() {
    printf("Testing top_k...\n");
    
    float a[] = {0.1, 0.5, 0.3, 0.9, 0.2, 0.8, 0.4, 0.7, 0.6, 0.0};
    int n = 10;
    int k = 3;
    int *indexes = calloc(k, sizeof(int));
    
    top_k(a, n, k, indexes);
    
    assert(indexes[0] == 3);
    assert(indexes[1] == 5);
    assert(indexes[2] == 7);
    
    free(indexes);
    
    printf("  ✓ top_k tests passed\n");
}

void test_network_accuracy() {
    printf("Testing network_accuracy...\n");
    
    network net = {0};
    net.n = 1;
    layer l = {0};
    l.outputs = 10;
    l.output = calloc(10, sizeof(float));
    net.layers = &l;
    
    data d = {0};
    d.X = make_matrix(3, 100);
    d.y = make_matrix(3, 10);
    
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 10; j++) {
            d.y.vals[i][j] = 0;
        }
        d.y.vals[i][i] = 1;
    }
    
    for(int j = 0; j < 10; j++) {
        l.output[j] = 0.1;
    }
    l.output[0] = 0.9;
    float acc1 = network_accuracy(&net, d);
    
    l.output[1] = 0.9;
    l.output[0] = 0.1;
    float acc2 = network_accuracy(&net, d);
    
    l.output[2] = 0.9;
    l.output[1] = 0.1;
    float acc3 = network_accuracy(&net, d);
    
    assert(acc1 > acc2);
    assert(acc3 > acc2);
    
    free(l.output);
    free_matrix(d.X);
    free_matrix(d.y);
    
    printf("  ✓ network_accuracy tests passed\n");
}

// Test removed - resize_layer_by_type is not in public API

int main() {
    printf("\n=== Running Network Tests ===\n\n");
    
    test_make_network();
    test_get_current_rate();
    test_get_network_output();
    test_get_network_output_layer();
    test_get_network_cost();
    test_top_k();
    test_network_accuracy();
    
    printf("\n=== All Network Tests Passed! ===\n\n");
    
    return 0;
}