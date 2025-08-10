#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "../include/darknet.h"

// Test basic matrix operations
void test_matrix_operations() {
    printf("Testing matrix operations...\n");
    
    matrix m = make_matrix(3, 4);
    assert(m.rows == 3);
    assert(m.cols == 4);
    assert(m.vals != NULL);
    
    for(int i = 0; i < m.rows; i++) {
        for(int j = 0; j < m.cols; j++) {
            m.vals[i][j] = i * m.cols + j;
        }
    }
    
    assert(m.vals[0][0] == 0);
    assert(m.vals[1][2] == 6);
    assert(m.vals[2][3] == 11);
    
    free_matrix(m);
    
    printf("  ✓ matrix operations tests passed\n");
}

// Test image operations
void test_image_operations() {
    printf("Testing image operations...\n");
    
    image im = make_image(640, 480, 3);
    
    assert(im.w == 640);
    assert(im.h == 480);
    assert(im.c == 3);
    assert(im.data != NULL);
    
    // Test pixel operations
    set_pixel(im, 50, 50, 0, 0.5);
    set_pixel(im, 50, 50, 1, 0.6);
    set_pixel(im, 50, 50, 2, 0.7);
    
    float r = get_pixel(im, 50, 50, 0);
    float g = get_pixel(im, 50, 50, 1);
    float b = get_pixel(im, 50, 50, 2);
    
    assert(fabs(r - 0.5) < 0.001);
    assert(fabs(g - 0.6) < 0.001);
    assert(fabs(b - 0.7) < 0.001);
    
    free_image(im);
    
    printf("  ✓ image operations tests passed\n");
}

// Test box operations
void test_box_operations() {
    printf("Testing box operations...\n");
    
    box a = {0.5, 0.5, 0.4, 0.4};
    box b = {0.5, 0.5, 0.4, 0.4};
    
    float iou = box_iou(a, b);
    assert(iou > 0.99 && iou < 1.01);
    
    box c = {0.0, 0.0, 0.2, 0.2};
    box d = {1.0, 1.0, 0.2, 0.2};
    
    iou = box_iou(c, d);
    assert(iou < 0.01);
    
    printf("  ✓ box operations tests passed\n");
}

// Test network creation
void test_network_creation() {
    printf("Testing network creation...\n");
    
    network *net = make_network(5);
    assert(net != NULL);
    assert(net->n == 5);
    assert(net->layers != NULL);
    
    free_network(net);
    
    printf("  ✓ network creation tests passed\n");
}

// Test list operations
void test_list_operations() {
    printf("Testing list operations...\n");
    
    list *l = make_list();
    assert(l != NULL);
    assert(l->size == 0);
    
    list_insert(l, "first");
    list_insert(l, "second");
    list_insert(l, "third");
    
    assert(l->size == 3);
    
    char **array = (char **)list_to_array(l);
    assert(array != NULL);
    
    free(array);
    free_list(l);
    
    printf("  ✓ list operations tests passed\n");
}

// Test data structures
void test_data_structures() {
    printf("Testing data structures...\n");
    
    data d = {0};
    d.X = make_matrix(32, 3*224*224);
    d.y = make_matrix(32, 1000);
    
    assert(d.X.rows == 32);
    assert(d.X.cols == 3*224*224);
    assert(d.y.rows == 32);
    assert(d.y.cols == 1000);
    
    free_data(d);
    
    printf("  ✓ data structures tests passed\n");
}

// Test detection structures
void test_detection_structures() {
    printf("Testing detection structures...\n");
    
    detection *dets = calloc(3, sizeof(detection));
    
    dets[0].bbox = (box){0.5, 0.5, 0.3, 0.3};
    dets[0].objectness = 0.9;
    dets[0].classes = 2;
    dets[0].prob = calloc(2, sizeof(float));
    dets[0].prob[0] = 0.9;
    dets[0].prob[1] = 0.1;
    
    assert(dets[0].objectness > 0.89);
    assert(dets[0].prob[0] > 0.89);
    
    do_nms_sort(dets, 1, 2, 0.5);
    
    free(dets[0].prob);
    free(dets);
    
    printf("  ✓ detection structures tests passed\n");
}

int main() {
    printf("\n========================================\n");
    printf("     Darknet Public API Test Suite\n");
    printf("========================================\n\n");
    
    test_matrix_operations();
    test_image_operations();
    test_box_operations();
    test_network_creation();
    test_list_operations();
    test_data_structures();
    test_detection_structures();
    
    printf("\n========================================\n");
    printf("     All Public API Tests Passed!\n");
    printf("========================================\n\n");
    
    return 0;
}