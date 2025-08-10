#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/data.h"
#include "../src/utils.h"

// Test removed - normalize_load_args is static/internal function

// Test removed - get_random_paths and find_replace_paths are not in public API

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

void test_get_labels() {
    printf("Testing get_labels...\n");
    
    FILE *fp = fopen("/tmp/test_labels.txt", "w");
    assert(fp != NULL);
    fprintf(fp, "label1\n");
    fprintf(fp, "label2\n");
    fprintf(fp, "label3\n");
    fclose(fp);
    
    char **labels = get_labels("/tmp/test_labels.txt");
    assert(labels != NULL);
    assert(strcmp(labels[0], "label1") == 0);
    assert(strcmp(labels[1], "label2") == 0);
    assert(strcmp(labels[2], "label3") == 0);
    
    for(int i = 0; i < 3; i++) {
        free(labels[i]);
    }
    free(labels);
    remove("/tmp/test_labels.txt");
    
    printf("  ✓ get_labels tests passed\n");
}

void test_fill_truth() {
    printf("Testing fill_truth...\n");
    
    char *path = "/data/images/test.jpg";
    char **labels = calloc(3, sizeof(char*));
    labels[0] = copy_string("cat");
    labels[1] = copy_string("dog");
    labels[2] = copy_string("bird");
    
    float *truth = calloc(3, sizeof(float));
    
    char labelpath[256];
    find_replace(path, "images", "labels", labelpath);
    char labelpath2[256];
    find_replace(labelpath, ".jpg", ".txt", labelpath2);
    
    FILE *fp = fopen("/tmp/test_truth.txt", "w");
    assert(fp != NULL);
    fprintf(fp, "1\n");
    fclose(fp);
    
    fill_truth("/tmp/test_truth.txt", labels, 3, truth);
    
    assert(truth[0] == 0.0);
    assert(truth[1] == 1.0);
    assert(truth[2] == 0.0);
    
    free(truth);
    for(int i = 0; i < 3; i++) {
        free(labels[i]);
    }
    free(labels);
    remove("/tmp/test_truth.txt");
    
    printf("  ✓ fill_truth tests passed\n");
}

void test_data_batch_allocation() {
    printf("Testing data batch allocation...\n");
    
    data d = {0};
    d.X = make_matrix(32, 3*224*224);
    d.y = make_matrix(32, 1000);
    
    assert(d.X.rows == 32);
    assert(d.X.cols == 3*224*224);
    assert(d.y.rows == 32);
    assert(d.y.cols == 1000);
    
    free_data(d);
    
    printf("  ✓ data batch allocation tests passed\n");
}

int main() {
    printf("\n=== Running Data Tests ===\n\n");
    
    test_matrix_operations();
    test_get_labels();
    test_fill_truth();
    test_data_batch_allocation();
    
    printf("\n=== All Data Tests Passed! ===\n\n");
    
    return 0;
}