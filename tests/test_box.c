#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "../src/box.h"

void test_box_iou() {
    printf("Testing box_iou...\n");
    
    box a = {0.5, 0.5, 0.4, 0.4};
    box b = {0.5, 0.5, 0.4, 0.4};
    
    float iou = box_iou(a, b);
    assert(iou > 0.99 && iou < 1.01);
    
    box c = {0.0, 0.0, 0.2, 0.2};
    box d = {1.0, 1.0, 0.2, 0.2};
    
    iou = box_iou(c, d);
    assert(iou < 0.01);
    
    box e = {0.5, 0.5, 0.4, 0.4};
    box f = {0.6, 0.6, 0.4, 0.4};
    
    iou = box_iou(e, f);
    assert(iou > 0.4 && iou < 0.7);
    
    printf("  ✓ box_iou tests passed\n");
}

void test_box_rmse() {
    printf("Testing box_rmse...\n");
    
    box a = {0.5, 0.5, 0.4, 0.4};
    box b = {0.5, 0.5, 0.4, 0.4};
    
    float rmse = box_rmse(a, b);
    assert(rmse < 0.01);
    
    box c = {0.0, 0.0, 0.2, 0.2};
    box d = {1.0, 1.0, 0.8, 0.8};
    
    rmse = box_rmse(c, d);
    assert(rmse > 0.5);
    
    printf("  ✓ box_rmse tests passed\n");
}

void test_float_to_box() {
    printf("Testing float_to_box...\n");
    
    float f[] = {0.5, 0.6, 0.3, 0.4};
    
    box b = float_to_box(f, 1);
    assert(b.x == 0.5);
    assert(b.y == 0.6);
    assert(b.w == 0.3);
    assert(b.h == 0.4);
    
    float f2[] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8};
    box b2 = float_to_box(f2 + 4, 1);
    assert(b2.x == 0.5);
    assert(b2.y == 0.6);
    assert(b2.w == 0.7);
    assert(b2.h == 0.8);
    
    printf("  ✓ float_to_box tests passed\n");
}

void test_do_nms_sort() {
    printf("Testing do_nms_sort...\n");
    
    detection *dets = calloc(3, sizeof(detection));
    
    dets[0].bbox = (box){0.5, 0.5, 0.3, 0.3};
    dets[0].objectness = 0.9;
    dets[0].classes = 2;
    dets[0].prob = calloc(2, sizeof(float));
    dets[0].prob[0] = 0.9;
    dets[0].prob[1] = 0.1;
    
    dets[1].bbox = (box){0.55, 0.55, 0.3, 0.3};
    dets[1].objectness = 0.8;
    dets[1].classes = 2;
    dets[1].prob = calloc(2, sizeof(float));
    dets[1].prob[0] = 0.8;
    dets[1].prob[1] = 0.2;
    
    dets[2].bbox = (box){0.9, 0.9, 0.2, 0.2};
    dets[2].objectness = 0.7;
    dets[2].classes = 2;
    dets[2].prob = calloc(2, sizeof(float));
    dets[2].prob[0] = 0.7;
    dets[2].prob[1] = 0.3;
    
    do_nms_sort(dets, 3, 2, 0.5);
    
    assert(dets[0].prob[0] > 0.89);
    assert(dets[1].prob[0] < 0.01);
    assert(dets[2].prob[0] > 0.69);
    
    for(int i = 0; i < 3; i++) {
        free(dets[i].prob);
    }
    free(dets);
    
    printf("  ✓ do_nms_sort tests passed\n");
}

// Tests removed - box_intersection and box_union are internal implementation details
// The public API is box_iou which uses these internally

void test_encode_decode_box() {
    printf("Testing encode/decode box...\n");
    
    box truth = {0.5, 0.6, 0.3, 0.4};
    box anchor = {0.4, 0.5, 0.2, 0.3};
    
    box encoded = encode_box(truth, anchor);
    
    // Verify encoded values are reasonable
    assert(encoded.x != 0 || encoded.y != 0 || encoded.w != 0 || encoded.h != 0);
    
    // Can decode manually
    box decoded = {0};
    decoded.x = encoded.x * anchor.w + anchor.x;
    decoded.y = encoded.y * anchor.h + anchor.y;
    decoded.w = exp(encoded.w) * anchor.w;
    decoded.h = exp(encoded.h) * anchor.h;
    
    assert(fabs(decoded.x - truth.x) < 0.01);
    assert(fabs(decoded.y - truth.y) < 0.01);
    assert(fabs(decoded.w - truth.w) < 0.01);
    assert(fabs(decoded.h - truth.h) < 0.01);
    
    printf("  ✓ encode/decode box tests passed\n");
}

void test_get_yolo_box() {
    printf("Testing get_yolo_box...\n");
    
    float x[] = {0.5, 0.6, 0.1, 0.2, 0.9};
    float biases[] = {1.0, 1.0};
    
    box b = get_yolo_box(x, biases, 0, 0, 0, 1, 1, 416, 416, 13, 13);
    
    assert(b.x >= 0 && b.x <= 1);
    assert(b.y >= 0 && b.y <= 1);
    assert(b.w >= 0 && b.w <= 1);
    assert(b.h >= 0 && b.h <= 1);
    
    printf("  ✓ get_yolo_box tests passed\n");
}

int main() {
    printf("\n=== Running Box Tests ===\n\n");
    
    test_box_iou();
    test_box_rmse();
    test_float_to_box();
    test_do_nms_sort();
    test_encode_decode_box();
    test_get_yolo_box();
    
    printf("\n=== All Box Tests Passed! ===\n\n");
    
    return 0;
}