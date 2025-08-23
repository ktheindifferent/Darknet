#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include "../src/utils.h"
#include "../src/safe_math.h"
#include "../src/box.h"
#include "../src/yolo_layer.h"

#define TOLERANCE 1e-6

// External functions from utils.c
extern float mean_array(float *a, int n);
extern float variance_array(float *a, int n);
extern float mse_array(float *a, int n);
extern void normalize_array(float *a, int n);
extern void mean_arrays(float **a, int n, int els, float *avg);

// External functions from yolo_layer.c
extern box get_yolo_box(float *x, float *biases, int n, int index, int i, int j, int lw, int lh, int w, int h, int stride);
extern void correct_yolo_boxes(detection *dets, int n, int w, int h, int netw, int neth, int relative);

void test_mean_array_division_by_zero() {
    printf("Testing mean_array with n=0...\n");
    
    float arr[] = {1.0, 2.0, 3.0};
    
    // Test with n=0 (should return 0.0 instead of crashing)
    float result = mean_array(arr, 0);
    assert(fabs(result - 0.0) < TOLERANCE);
    
    // Test with negative n (should return 0.0)
    result = mean_array(arr, -5);
    assert(fabs(result - 0.0) < TOLERANCE);
    
    // Test normal case
    result = mean_array(arr, 3);
    assert(fabs(result - 2.0) < TOLERANCE);
    
    printf("  ✓ mean_array division protection passed\n");
}

void test_variance_array_division_by_zero() {
    printf("Testing variance_array with n=0...\n");
    
    float arr[] = {1.0, 2.0, 3.0};
    
    // Test with n=0 (should return 0.0 instead of crashing)
    float result = variance_array(arr, 0);
    assert(fabs(result - 0.0) < TOLERANCE);
    
    // Test with negative n
    result = variance_array(arr, -1);
    assert(fabs(result - 0.0) < TOLERANCE);
    
    // Test normal case
    result = variance_array(arr, 3);
    assert(result >= 0); // Variance should be non-negative
    
    printf("  ✓ variance_array division protection passed\n");
}

void test_mse_array_division_by_zero() {
    printf("Testing mse_array with n=0...\n");
    
    float arr[] = {1.0, 2.0, 3.0};
    
    // Test with n=0 (should return 0.0 instead of crashing)
    float result = mse_array(arr, 0);
    assert(fabs(result - 0.0) < TOLERANCE);
    
    // Test with negative n
    result = mse_array(arr, -10);
    assert(fabs(result - 0.0) < TOLERANCE);
    
    // Test normal case
    result = mse_array(arr, 3);
    assert(result >= 0); // MSE should be non-negative
    
    printf("  ✓ mse_array division protection passed\n");
}

void test_normalize_array_zero_variance() {
    printf("Testing normalize_array with zero variance...\n");
    
    // Array with all same values (zero variance)
    float arr1[] = {5.0, 5.0, 5.0, 5.0};
    float arr1_copy[4];
    memcpy(arr1_copy, arr1, sizeof(arr1));
    
    normalize_array(arr1, 4);
    
    // Should not modify array when variance is zero
    for (int i = 0; i < 4; i++) {
        assert(fabs(arr1[i] - arr1_copy[i]) < TOLERANCE);
    }
    
    // Test with normal variance
    float arr2[] = {1.0, 2.0, 3.0, 4.0};
    normalize_array(arr2, 4);
    
    // After normalization, mean should be close to 0
    float mean = mean_array(arr2, 4);
    assert(fabs(mean) < TOLERANCE);
    
    printf("  ✓ normalize_array division protection passed\n");
}

void test_mean_arrays_division_by_zero() {
    printf("Testing mean_arrays with n=0...\n");
    
    float row1[] = {1.0, 2.0, 3.0};
    float row2[] = {4.0, 5.0, 6.0};
    float *arrays[] = {row1, row2};
    float avg[3];
    
    // Test with n=0
    mean_arrays(arrays, 0, 3, avg);
    for (int i = 0; i < 3; i++) {
        assert(fabs(avg[i] - 0.0) < TOLERANCE);
    }
    
    // Test with els=0
    mean_arrays(arrays, 2, 0, avg);
    // Should not crash
    
    // Test normal case
    mean_arrays(arrays, 2, 3, avg);
    assert(fabs(avg[0] - 2.5) < TOLERANCE);
    assert(fabs(avg[1] - 3.5) < TOLERANCE);
    assert(fabs(avg[2] - 4.5) < TOLERANCE);
    
    printf("  ✓ mean_arrays division protection passed\n");
}

void test_get_yolo_box_division_by_zero() {
    printf("Testing get_yolo_box with zero dimensions...\n");
    
    float x[4] = {0.5, 0.5, 1.0, 1.0};
    float biases[4] = {10.0, 13.0, 16.0, 30.0};
    
    // Test with zero dimensions (should not crash)
    box b = get_yolo_box(x, biases, 0, 0, 1, 1, 0, 0, 0, 0, 1);
    assert(isfinite(b.x) || b.x == 0.0);
    assert(isfinite(b.y) || b.y == 0.0);
    assert(isfinite(b.w) || b.w == 0.0);
    assert(isfinite(b.h) || b.h == 0.0);
    
    // Test with normal dimensions
    b = get_yolo_box(x, biases, 0, 0, 1, 1, 10, 10, 100, 100, 1);
    assert(isfinite(b.x));
    assert(isfinite(b.y));
    assert(isfinite(b.w));
    assert(isfinite(b.h));
    
    printf("  ✓ get_yolo_box division protection passed\n");
}

void test_correct_yolo_boxes_division_by_zero() {
    printf("Testing correct_yolo_boxes with zero dimensions...\n");
    
    detection det;
    det.bbox.x = 0.5;
    det.bbox.y = 0.5;
    det.bbox.w = 0.1;
    det.bbox.h = 0.1;
    det.prob = calloc(1, sizeof(float));
    det.prob[0] = 0.9;
    det.classes = 1;
    
    detection dets[] = {det};
    
    // Test with zero width/height (should not crash)
    correct_yolo_boxes(dets, 1, 0, 0, 100, 100, 0);
    assert(isfinite(dets[0].bbox.x));
    assert(isfinite(dets[0].bbox.y));
    assert(isfinite(dets[0].bbox.w));
    assert(isfinite(dets[0].bbox.h));
    
    // Test with zero network dimensions
    dets[0] = det;  // Reset
    correct_yolo_boxes(dets, 1, 100, 100, 0, 0, 0);
    assert(isfinite(dets[0].bbox.x));
    assert(isfinite(dets[0].bbox.y));
    assert(isfinite(dets[0].bbox.w));
    assert(isfinite(dets[0].bbox.h));
    
    // Test normal case
    dets[0] = det;  // Reset
    correct_yolo_boxes(dets, 1, 640, 480, 416, 416, 0);
    assert(isfinite(dets[0].bbox.x));
    assert(isfinite(dets[0].bbox.y));
    assert(isfinite(dets[0].bbox.w));
    assert(isfinite(dets[0].bbox.h));
    
    free(det.prob);
    
    printf("  ✓ correct_yolo_boxes division protection passed\n");
}

void test_shuffle_with_edge_cases() {
    printf("Testing shuffle with edge cases...\n");
    
    int arr1[] = {1, 2, 3, 4, 5};
    int arr_empty[0];
    int arr_single[] = {42};
    
    // Test with n=0 (should not crash)
    shuffle(arr_empty, 0, sizeof(int));
    
    // Test with n=1 (should not change)
    int original = arr_single[0];
    shuffle(arr_single, 1, sizeof(int));
    assert(arr_single[0] == original);
    
    // Test normal case (should work without division by zero)
    shuffle(arr1, 5, sizeof(int));
    // Array should be shuffled (we can't test exact output due to randomness)
    
    printf("  ✓ shuffle edge case tests passed\n");
}

int main() {
    printf("\n=== Running Division Protection Tests ===\n\n");
    
    printf("Utils.c division protection tests:\n");
    test_mean_array_division_by_zero();
    test_variance_array_division_by_zero();
    test_mse_array_division_by_zero();
    test_normalize_array_zero_variance();
    test_mean_arrays_division_by_zero();
    test_shuffle_with_edge_cases();
    
    printf("\nYOLO layer division protection tests:\n");
    test_get_yolo_box_division_by_zero();
    test_correct_yolo_boxes_division_by_zero();
    
    printf("\n=== All Division Protection Tests Passed! ===\n\n");
    return 0;
}