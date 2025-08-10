#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "../src/image.h"

void test_make_image() {
    printf("Testing make_image...\n");
    
    image im = make_image(640, 480, 3);
    
    assert(im.w == 640);
    assert(im.h == 480);
    assert(im.c == 3);
    assert(im.data != NULL);
    
    int size = im.w * im.h * im.c;
    for(int i = 0; i < size; i++) {
        assert(im.data[i] == 0.0);
    }
    
    free_image(im);
    
    printf("  ✓ make_image tests passed\n");
}

void test_resize_image() {
    printf("Testing resize_image...\n");
    
    image orig = make_image(100, 100, 3);
    
    for(int i = 0; i < orig.w * orig.h * orig.c; i++) {
        orig.data[i] = (float)i / (orig.w * orig.h * orig.c);
    }
    
    image resized = resize_image(orig, 50, 50);
    
    assert(resized.w == 50);
    assert(resized.h == 50);
    assert(resized.c == 3);
    assert(resized.data != NULL);
    
    free_image(orig);
    free_image(resized);
    
    printf("  ✓ resize_image tests passed\n");
}

void test_letterbox_image() {
    printf("Testing letterbox_image...\n");
    
    image orig = make_image(640, 480, 3);
    
    for(int i = 0; i < orig.w * orig.h * orig.c; i++) {
        orig.data[i] = 0.5;
    }
    
    image boxed = letterbox_image(orig, 416, 416);
    
    assert(boxed.w == 416);
    assert(boxed.h == 416);
    assert(boxed.c == 3);
    
    free_image(orig);
    free_image(boxed);
    
    printf("  ✓ letterbox_image tests passed\n");
}

void test_copy_image() {
    printf("Testing copy_image...\n");
    
    image orig = make_image(100, 100, 3);
    
    for(int i = 0; i < orig.w * orig.h * orig.c; i++) {
        orig.data[i] = (float)i / 1000.0;
    }
    
    image copy = copy_image(orig);
    
    assert(copy.w == orig.w);
    assert(copy.h == orig.h);
    assert(copy.c == orig.c);
    assert(copy.data != orig.data);
    
    for(int i = 0; i < orig.w * orig.h * orig.c; i++) {
        assert(fabs(copy.data[i] - orig.data[i]) < 0.001);
    }
    
    free_image(orig);
    free_image(copy);
    
    printf("  ✓ copy_image tests passed\n");
}

void test_get_set_pixel() {
    printf("Testing get/set_pixel...\n");
    
    image im = make_image(100, 100, 3);
    
    set_pixel(im, 50, 50, 0, 0.5);
    set_pixel(im, 50, 50, 1, 0.6);
    set_pixel(im, 50, 50, 2, 0.7);
    
    float r = get_pixel(im, 50, 50, 0);
    float g = get_pixel(im, 50, 50, 1);
    float b = get_pixel(im, 50, 50, 2);
    
    assert(fabs(r - 0.5) < 0.001);
    assert(fabs(g - 0.6) < 0.001);
    assert(fabs(b - 0.7) < 0.001);
    
    float outside = get_pixel(im, 200, 200, 0);
    assert(outside == 0.0);
    
    free_image(im);
    
    printf("  ✓ get/set_pixel tests passed\n");
}

void test_rgb_to_hsv() {
    printf("Testing rgb_to_hsv...\n");
    
    image rgb = make_image(2, 2, 3);
    
    set_pixel(rgb, 0, 0, 0, 1.0);
    set_pixel(rgb, 0, 0, 1, 0.0);
    set_pixel(rgb, 0, 0, 2, 0.0);
    
    set_pixel(rgb, 1, 0, 0, 0.0);
    set_pixel(rgb, 1, 0, 1, 1.0);
    set_pixel(rgb, 1, 0, 2, 0.0);
    
    rgb_to_hsv(rgb);
    
    float h1 = get_pixel(rgb, 0, 0, 0);
    float s1 = get_pixel(rgb, 0, 0, 1);
    float v1 = get_pixel(rgb, 0, 0, 2);
    
    assert(fabs(h1 - 0.0) < 0.01);
    assert(fabs(s1 - 1.0) < 0.01);
    assert(fabs(v1 - 1.0) < 0.01);
    
    free_image(rgb);
    
    printf("  ✓ rgb_to_hsv tests passed\n");
}

void test_flip_image() {
    printf("Testing flip_image...\n");
    
    image orig = make_image(3, 3, 1);
    
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            set_pixel(orig, j, i, 0, i * 3 + j);
        }
    }
    
    flip_image(orig);
    
    assert(fabs(get_pixel(orig, 0, 0, 0) - 2) < 0.001);
    assert(fabs(get_pixel(orig, 1, 0, 0) - 1) < 0.001);
    assert(fabs(get_pixel(orig, 2, 0, 0) - 0) < 0.001);
    
    free_image(orig);
    
    printf("  ✓ flip_image tests passed\n");
}

void test_constrain_image() {
    printf("Testing constrain_image...\n");
    
    image im = make_image(10, 10, 3);
    
    for(int i = 0; i < im.w * im.h * im.c; i++) {
        im.data[i] = ((float)i - 50) / 50.0;
    }
    
    constrain_image(im);
    
    for(int i = 0; i < im.w * im.h * im.c; i++) {
        assert(im.data[i] >= 0.0);
        assert(im.data[i] <= 1.0);
    }
    
    free_image(im);
    
    printf("  ✓ constrain_image tests passed\n");
}

void test_grayscale_image() {
    printf("Testing grayscale_image...\n");
    
    image color = make_image(10, 10, 3);
    
    for(int i = 0; i < 10; i++) {
        for(int j = 0; j < 10; j++) {
            set_pixel(color, j, i, 0, 0.3);
            set_pixel(color, j, i, 1, 0.5);
            set_pixel(color, j, i, 2, 0.2);
        }
    }
    
    image gray = grayscale_image(color);
    
    assert(gray.w == 10);
    assert(gray.h == 10);
    assert(gray.c == 1);
    
    float expected = 0.299 * 0.3 + 0.587 * 0.5 + 0.114 * 0.2;
    for(int i = 0; i < gray.w * gray.h; i++) {
        assert(fabs(gray.data[i] - expected) < 0.01);
    }
    
    free_image(color);
    free_image(gray);
    
    printf("  ✓ grayscale_image tests passed\n");
}

void test_threshold_image() {
    printf("Testing threshold_image...\n");
    
    image im = make_image(10, 10, 1);
    
    for(int i = 0; i < im.w * im.h; i++) {
        im.data[i] = (float)i / 100.0;
    }
    
    threshold_image(im, 0.5);
    
    for(int i = 0; i < im.w * im.h; i++) {
        if(i < 50) {
            assert(im.data[i] == 0.0);
        } else {
            assert(im.data[i] == 1.0);
        }
    }
    
    free_image(im);
    
    printf("  ✓ threshold_image tests passed\n");
}

int main() {
    printf("\n=== Running Image Tests ===\n\n");
    
    test_make_image();
    test_resize_image();
    test_letterbox_image();
    test_copy_image();
    test_get_set_pixel();
    test_rgb_to_hsv();
    test_flip_image();
    test_constrain_image();
    test_grayscale_image();
    test_threshold_image();
    
    printf("\n=== All Image Tests Passed! ===\n\n");
    
    return 0;
}