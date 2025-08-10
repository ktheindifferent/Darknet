#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/utils.h"

void test_find_int_arg() {
    printf("Testing find_int_arg...\n");
    
    char *argv[] = {"program", "-batch", "32", "-gpu", "1"};
    int argc = 5;
    
    int batch = find_int_arg(argc, argv, "-batch", 1);
    assert(batch == 32);
    
    int gpu = find_int_arg(argc, argv, "-gpu", 0);
    assert(gpu == 1);
    
    int missing = find_int_arg(argc, argv, "-missing", 99);
    assert(missing == 99);
    
    printf("  ✓ find_int_arg tests passed\n");
}

void test_find_float_arg() {
    printf("Testing find_float_arg...\n");
    
    char *argv[] = {"program", "-lr", "0.001", "-momentum", "0.9"};
    int argc = 5;
    
    float lr = find_float_arg(argc, argv, "-lr", 0.1);
    assert(lr > 0.0009 && lr < 0.0011);
    
    float momentum = find_float_arg(argc, argv, "-momentum", 0.5);
    assert(momentum > 0.89 && momentum < 0.91);
    
    float missing = find_float_arg(argc, argv, "-missing", 1.5);
    assert(missing > 1.49 && missing < 1.51);
    
    printf("  ✓ find_float_arg tests passed\n");
}

void test_find_arg() {
    printf("Testing find_arg...\n");
    
    char *argv[] = {"program", "-gpu", "1", "-nogpu", "-batch", "32"};
    int argc = 6;
    
    int gpu_idx = find_arg(argc, argv, "-gpu");
    assert(gpu_idx == 1);
    
    int nogpu_idx = find_arg(argc, argv, "-nogpu");
    assert(nogpu_idx == 3);
    
    int missing_idx = find_arg(argc, argv, "-missing");
    assert(missing_idx == -1);
    
    printf("  ✓ find_arg tests passed\n");
}

void test_find_char_arg() {
    printf("Testing find_char_arg...\n");
    
    char *argv[] = {"program", "-config", "yolo.cfg", "-weights", "yolo.weights"};
    int argc = 5;
    
    char *config = find_char_arg(argc, argv, "-config", "default.cfg");
    assert(strcmp(config, "yolo.cfg") == 0);
    
    char *weights = find_char_arg(argc, argv, "-weights", "default.weights");
    assert(strcmp(weights, "yolo.weights") == 0);
    
    char *missing = find_char_arg(argc, argv, "-missing", "default");
    assert(strcmp(missing, "default") == 0);
    
    printf("  ✓ find_char_arg tests passed\n");
}

void test_strip() {
    printf("Testing strip...\n");
    
    char test1[] = "  hello  ";
    strip(test1);
    assert(strcmp(test1, "hello") == 0);
    
    char test2[] = "\t\nworld\r\n";
    strip(test2);
    assert(strcmp(test2, "world") == 0);
    
    char test3[] = "nowhitespace";
    strip(test3);
    assert(strcmp(test3, "nowhitespace") == 0);
    
    char test4[] = "   ";
    strip(test4);
    assert(strcmp(test4, "") == 0);
    
    printf("  ✓ strip tests passed\n");
}

void test_fgetl() {
    printf("Testing fgetl...\n");
    
    FILE *fp = fopen("/tmp/test_fgetl.txt", "w");
    assert(fp != NULL);
    fprintf(fp, "line1\nline2\r\nline3");
    fclose(fp);
    
    fp = fopen("/tmp/test_fgetl.txt", "r");
    assert(fp != NULL);
    
    char *line1 = fgetl(fp);
    assert(strcmp(line1, "line1") == 0);
    free(line1);
    
    char *line2 = fgetl(fp);
    assert(strcmp(line2, "line2") == 0);
    free(line2);
    
    char *line3 = fgetl(fp);
    assert(strcmp(line3, "line3") == 0);
    free(line3);
    
    char *eof = fgetl(fp);
    assert(eof == NULL);
    
    fclose(fp);
    remove("/tmp/test_fgetl.txt");
    
    printf("  ✓ fgetl tests passed\n");
}

void test_copy_string() {
    printf("Testing copy_string...\n");
    
    char *original = "Hello, World!";
    char *copy = copy_string(original);
    
    assert(copy != NULL);
    assert(strcmp(copy, original) == 0);
    assert(copy != original);
    
    free(copy);
    
    char *null_copy = copy_string(NULL);
    assert(null_copy == NULL);
    
    printf("  ✓ copy_string tests passed\n");
}

void test_list_to_array() {
    printf("Testing list_to_array...\n");
    
    list *l = make_list();
    list_insert(l, "first");
    list_insert(l, "second");
    list_insert(l, "third");
    
    char **array = (char **)list_to_array(l);
    
    assert(strcmp(array[0], "third") == 0);
    assert(strcmp(array[1], "second") == 0);
    assert(strcmp(array[2], "first") == 0);
    
    free(array);
    free_list(l);
    
    printf("  ✓ list_to_array tests passed\n");
}

void test_read_intlist() {
    printf("Testing read_intlist...\n");
    
    FILE *fp = fopen("/tmp/test_intlist.txt", "w");
    assert(fp != NULL);
    fprintf(fp, "1,2,3,4,5\n");
    fprintf(fp, "10,20,30\n");
    fclose(fp);
    
    int *count = calloc(1, sizeof(int));
    int *list = read_intlist("/tmp/test_intlist.txt", count, 0);
    
    assert(*count == 8);
    assert(list[0] == 1);
    assert(list[4] == 5);
    assert(list[5] == 10);
    assert(list[7] == 30);
    
    free(list);
    free(count);
    remove("/tmp/test_intlist.txt");
    
    printf("  ✓ read_intlist tests passed\n");
}

void test_sec() {
    printf("Testing sec (timing)...\n");
    
    double t1 = sec(clock());
    
    volatile int sum = 0;
    for(int i = 0; i < 1000000; i++) {
        sum += i;
    }
    
    double t2 = sec(clock());
    
    assert(t2 > t1);
    assert((t2 - t1) < 1.0);
    
    printf("  ✓ sec tests passed\n");
}

void test_find_replace() {
    printf("Testing find_replace...\n");
    
    char test1[256] = "hello world";
    char result1[256];
    find_replace(test1, "world", "universe", result1);
    assert(strcmp(result1, "hello universe") == 0);
    
    char test2[256] = "foo bar foo";
    char result2[256];
    find_replace(test2, "foo", "baz", result2);
    assert(strcmp(result2, "baz bar baz") == 0);
    
    char test3[256] = "no match here";
    char result3[256];
    find_replace(test3, "xyz", "abc", result3);
    assert(strcmp(result3, "no match here") == 0);
    
    printf("  ✓ find_replace tests passed\n");
}

int main() {
    printf("\n=== Running Utils Tests ===\n\n");
    
    test_find_int_arg();
    test_find_float_arg();
    test_find_arg();
    test_find_char_arg();
    test_strip();
    test_fgetl();
    test_copy_string();
    test_list_to_array();
    test_read_intlist();
    test_sec();
    test_find_replace();
    
    printf("\n=== All Utils Tests Passed! ===\n\n");
    
    return 0;
}