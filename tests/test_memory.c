#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <setjmp.h>
#include "../src/utils.h"
#include "../src/list.h"
#include "../src/matrix.h"

// Forward declaration for list_pop
void *list_pop(list *l);

// Signal handler for catching exit calls
static jmp_buf test_jump_buffer;
static int exit_called = 0;

void test_exit_handler(int sig) {
    if (sig == SIGABRT) {
        exit_called = 1;
        longjmp(test_jump_buffer, 1);
    }
}

// Test safe_malloc with valid size
void test_safe_malloc_valid() {
    printf("Testing safe_malloc with valid size... ");
    void *ptr = safe_malloc(100);
    assert(ptr != NULL);
    free(ptr);
    printf("PASSED\n");
}

// Test safe_calloc with valid parameters
void test_safe_calloc_valid() {
    printf("Testing safe_calloc with valid parameters... ");
    void *ptr = safe_calloc(10, sizeof(int));
    assert(ptr != NULL);
    
    // Verify memory is zeroed
    int *int_ptr = (int *)ptr;
    for (int i = 0; i < 10; i++) {
        assert(int_ptr[i] == 0);
    }
    
    free(ptr);
    printf("PASSED\n");
}

// Test safe_realloc with valid parameters
void test_safe_realloc_valid() {
    printf("Testing safe_realloc with valid parameters... ");
    void *ptr = safe_malloc(100);
    assert(ptr != NULL);
    
    ptr = safe_realloc(ptr, 200);
    assert(ptr != NULL);
    
    free(ptr);
    printf("PASSED\n");
}

// Test safe_malloc with zero size
void test_safe_malloc_zero() {
    printf("Testing safe_malloc with zero size... ");
    void *ptr = safe_malloc(0);
    // malloc(0) behavior is implementation-defined, could be NULL or valid pointer
    if (ptr) free(ptr);
    printf("PASSED\n");
}

// Test safe_calloc with zero parameters
void test_safe_calloc_zero() {
    printf("Testing safe_calloc with zero parameters... ");
    void *ptr = safe_calloc(0, 10);
    if (ptr) free(ptr);
    
    ptr = safe_calloc(10, 0);
    if (ptr) free(ptr);
    
    ptr = safe_calloc(0, 0);
    if (ptr) free(ptr);
    printf("PASSED\n");
}

// Test list creation with safe allocation
void test_list_safe_allocation() {
    printf("Testing list creation with safe allocation... ");
    list *l = make_list();
    assert(l != NULL);
    assert(l->size == 0);
    assert(l->front == NULL);
    assert(l->back == NULL);
    
    // Insert some items
    for (int i = 0; i < 10; i++) {
        int *val = safe_malloc(sizeof(int));
        *val = i;
        list_insert(l, val);
    }
    assert(l->size == 10);
    
    // Clean up
    while (l->size > 0) {
        void *val = list_pop(l);
        free(val);
    }
    free(l);
    printf("PASSED\n");
}

// Test matrix allocation
void test_matrix_safe_allocation() {
    printf("Testing matrix topk with safe allocation... ");
    
    // Create dummy matrices for testing
    matrix truth = {0};
    matrix guess = {0};
    truth.rows = 1;
    truth.cols = 10;
    guess.rows = 1;
    guess.cols = 10;
    
    // Allocate matrix data
    truth.vals = safe_calloc(1, sizeof(float*));
    truth.vals[0] = safe_calloc(10, sizeof(float));
    guess.vals = safe_calloc(1, sizeof(float*));
    guess.vals[0] = safe_calloc(10, sizeof(float));
    
    // Set some values
    truth.vals[0][5] = 1.0;
    for (int i = 0; i < 10; i++) {
        guess.vals[0][i] = (float)i;
    }
    
    // Test matrix_topk_accuracy which uses safe_calloc internally
    matrix_topk_accuracy(truth, guess, 3);
    
    // Clean up
    free(truth.vals[0]);
    free(truth.vals);
    free(guess.vals[0]);
    free(guess.vals);
    
    printf("PASSED\n");
}

// Test copy_string with safe allocation
void test_copy_string_safe() {
    printf("Testing copy_string with safe allocation... ");
    
    char *original = "Test string for copying";
    char *copy = copy_string(original);
    
    assert(copy != NULL);
    assert(strcmp(original, copy) == 0);
    assert(copy != original);  // Different pointers
    
    free(copy);
    printf("PASSED\n");
}

// Test multiple allocations to ensure consistency
void test_multiple_allocations() {
    printf("Testing multiple allocations... ");
    
    void *ptrs[100];
    
    // Allocate multiple blocks
    for (int i = 0; i < 100; i++) {
        ptrs[i] = safe_malloc((i + 1) * 10);
        assert(ptrs[i] != NULL);
    }
    
    // Free all blocks
    for (int i = 0; i < 100; i++) {
        free(ptrs[i]);
    }
    
    printf("PASSED\n");
}

// Test realloc growth pattern
void test_realloc_growth() {
    printf("Testing realloc growth pattern... ");
    
    size_t size = 10;
    char *ptr = safe_malloc(size);
    assert(ptr != NULL);
    
    // Fill with data
    for (size_t i = 0; i < size - 1; i++) {
        ptr[i] = 'A' + (i % 26);
    }
    ptr[size - 1] = '\0';
    
    // Grow the allocation several times
    for (int i = 0; i < 5; i++) {
        size *= 2;
        char *old_content = safe_malloc(strlen(ptr) + 1);
        strcpy(old_content, ptr);
        
        ptr = safe_realloc(ptr, size);
        assert(ptr != NULL);
        
        // Verify content preserved
        assert(strcmp(ptr, old_content) == 0);
        
        // Add more data
        size_t old_len = strlen(ptr);
        for (size_t j = old_len; j < size - 1 && j < old_len + 10; j++) {
            ptr[j] = 'A' + (j % 26);
        }
        ptr[size - 1] = '\0';
        
        free(old_content);
    }
    
    free(ptr);
    printf("PASSED\n");
}

int main() {
    printf("=== Memory Allocation Safety Tests ===\n\n");
    
    // Run all tests
    test_safe_malloc_valid();
    test_safe_calloc_valid();
    test_safe_realloc_valid();
    test_safe_malloc_zero();
    test_safe_calloc_zero();
    test_list_safe_allocation();
    test_matrix_safe_allocation();
    test_copy_string_safe();
    test_multiple_allocations();
    test_realloc_growth();
    
    printf("\n=== All tests PASSED ===\n");
    return 0;
}