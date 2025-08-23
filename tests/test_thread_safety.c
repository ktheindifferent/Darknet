#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include "../include/thread_sync.h"
#include "../src/network.h"
#include "../src/utils.h"
#include "../src/parser.h"
#include "../src/data.h"

#define NUM_THREADS 8
#define NUM_ITERATIONS 1000
#define TEST_LAYERS 10

// Test data structures
typedef struct {
    int thread_id;
    int iterations;
    void *shared_data;
    pthread_barrier_t *barrier;
} thread_test_args;

// Shared counters for testing
static atomic_int g_counter = 0;
static int g_unsafe_counter = 0;
static pthread_mutex_t g_test_mutex = PTHREAD_MUTEX_INITIALIZER;

// Test 1: Basic mutex operations
void *test_mutex_thread(void *arg) {
    thread_test_args *args = (thread_test_args*)arg;
    
    for (int i = 0; i < args->iterations; i++) {
        pthread_mutex_lock(&g_test_mutex);
        g_unsafe_counter++;
        pthread_mutex_unlock(&g_test_mutex);
        
        // Also test atomic operations
        atomic_fetch_add(&g_counter, 1);
    }
    
    return NULL;
}

void test_basic_mutex() {
    printf("Testing basic mutex operations...\n");
    
    pthread_t threads[NUM_THREADS];
    thread_test_args args[NUM_THREADS];
    
    g_counter = 0;
    g_unsafe_counter = 0;
    
    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].thread_id = i;
        args[i].iterations = NUM_ITERATIONS;
        pthread_create(&threads[i], NULL, test_mutex_thread, &args[i]);
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    int expected = NUM_THREADS * NUM_ITERATIONS;
    assert(g_unsafe_counter == expected);
    assert(atomic_load(&g_counter) == expected);
    
    printf("✓ Basic mutex test passed (counter: %d, atomic: %d)\n", 
           g_unsafe_counter, atomic_load(&g_counter));
}

// Test 2: Network sync context
void test_network_sync_context() {
    printf("Testing network sync context...\n");
    
    network_sync_context *ctx = create_network_sync_context(TEST_LAYERS);
    assert(ctx != NULL);
    assert(ctx->num_layers == TEST_LAYERS);
    assert(ctx->layer_mutexes != NULL);
    assert(atomic_load(&ctx->active_threads) == 0);
    assert(atomic_load(&ctx->seen_samples) == 0);
    
    // Test atomic operations
    atomic_fetch_add(&ctx->active_threads, 5);
    assert(atomic_load(&ctx->active_threads) == 5);
    
    atomic_fetch_sub(&ctx->active_threads, 3);
    assert(atomic_load(&ctx->active_threads) == 2);
    
    // Test seen_samples counter
    atomic_fetch_add(&ctx->seen_samples, 1000);
    assert(atomic_load(&ctx->seen_samples) == 1000);
    
    destroy_network_sync_context(ctx);
    printf("✓ Network sync context test passed\n");
}

// Test 3: Demo sync context
void test_demo_sync_context() {
    printf("Testing demo sync context...\n");
    
    demo_sync_context *ctx = create_demo_sync_context();
    assert(ctx != NULL);
    
    // Test atomic buffer index
    atomic_store(&ctx->buffer_index, 0);
    int idx = atomic_fetch_add(&ctx->buffer_index, 1);
    assert(idx == 0);
    assert(atomic_load(&ctx->buffer_index) == 1);
    
    // Test demo_running flag
    atomic_store(&ctx->demo_running, 1);
    assert(atomic_load(&ctx->demo_running) == 1);
    atomic_store(&ctx->demo_running, 0);
    assert(atomic_load(&ctx->demo_running) == 0);
    
    destroy_demo_sync_context(ctx);
    printf("✓ Demo sync context test passed\n");
}

// Test 4: Concurrent layer updates
typedef struct {
    network_sync_context *ctx;
    int layer_id;
    int thread_id;
    int *counter;
} layer_update_args;

void update_layer_counter(void *data) {
    int *counter = (int*)data;
    (*counter)++;
}

void *test_layer_update_thread(void *arg) {
    layer_update_args *args = (layer_update_args*)arg;
    
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        sync_network_update(args->ctx, args->layer_id, 
                           update_layer_counter, args->counter);
    }
    
    return NULL;
}

void test_concurrent_layer_updates() {
    printf("Testing concurrent layer updates...\n");
    
    network_sync_context *ctx = create_network_sync_context(TEST_LAYERS);
    int counters[TEST_LAYERS];
    memset(counters, 0, sizeof(counters));
    
    pthread_t threads[NUM_THREADS];
    layer_update_args args[NUM_THREADS];
    
    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].ctx = ctx;
        args[i].layer_id = i % TEST_LAYERS;
        args[i].thread_id = i;
        args[i].counter = &counters[args[i].layer_id];
        pthread_create(&threads[i], NULL, test_layer_update_thread, &args[i]);
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Verify counters
    for (int i = 0; i < TEST_LAYERS; i++) {
        int expected_count = 0;
        for (int j = 0; j < NUM_THREADS; j++) {
            if (j % TEST_LAYERS == i) {
                expected_count += NUM_ITERATIONS;
            }
        }
        assert(counters[i] == expected_count);
    }
    
    destroy_network_sync_context(ctx);
    printf("✓ Concurrent layer updates test passed\n");
}

// Test 5: Memory barriers
void *test_memory_barrier_thread(void *arg) {
    thread_test_args *args = (thread_test_args*)arg;
    pthread_barrier_t *barrier = args->barrier;
    
    // Phase 1: Write
    if (args->thread_id == 0) {
        atomic_store(&g_counter, 42);
        release_fence();  // Ensure write is visible
    }
    
    pthread_barrier_wait(barrier);
    
    // Phase 2: Read
    acquire_fence();  // Ensure we see the write
    int value = atomic_load(&g_counter);
    assert(value == 42);
    
    return NULL;
}

void test_memory_barriers() {
    printf("Testing memory barriers...\n");
    
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, NUM_THREADS);
    
    pthread_t threads[NUM_THREADS];
    thread_test_args args[NUM_THREADS];
    
    atomic_store(&g_counter, 0);
    
    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].thread_id = i;
        args[i].barrier = &barrier;
        pthread_create(&threads[i], NULL, test_memory_barrier_thread, &args[i]);
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    pthread_barrier_destroy(&barrier);
    printf("✓ Memory barriers test passed\n");
}

// Test 6: Race condition detection (deliberate race for testing)
typedef struct {
    int *shared_counter;
    int iterations;
    int use_mutex;
    pthread_mutex_t *mutex;
} race_test_args;

void *race_condition_thread(void *arg) {
    race_test_args *args = (race_test_args*)arg;
    
    for (int i = 0; i < args->iterations; i++) {
        if (args->use_mutex) {
            pthread_mutex_lock(args->mutex);
            (*args->shared_counter)++;
            pthread_mutex_unlock(args->mutex);
        } else {
            // Deliberate race condition for testing
            (*args->shared_counter)++;
        }
    }
    
    return NULL;
}

void test_race_condition_prevention() {
    printf("Testing race condition prevention...\n");
    
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
    
    // Test with mutex (should be safe)
    int safe_counter = 0;
    pthread_t threads[NUM_THREADS];
    race_test_args args[NUM_THREADS];
    
    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].shared_counter = &safe_counter;
        args[i].iterations = NUM_ITERATIONS;
        args[i].use_mutex = 1;
        args[i].mutex = &mutex;
        pthread_create(&threads[i], NULL, race_condition_thread, &args[i]);
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    assert(safe_counter == NUM_THREADS * NUM_ITERATIONS);
    
    pthread_mutex_destroy(&mutex);
    printf("✓ Race condition prevention test passed (safe counter: %d)\n", safe_counter);
}

// Test 7: Stress test with high contention
void *stress_test_thread(void *arg) {
    thread_test_args *args = (thread_test_args*)arg;
    sync_mutexes *sync = (sync_mutexes*)args->shared_data;
    
    for (int i = 0; i < args->iterations; i++) {
        // Simulate different operations with different mutexes
        int op = i % 5;
        switch(op) {
            case 0:
                pthread_mutex_lock(&sync->network_mutex);
                usleep(1);  // Simulate work
                pthread_mutex_unlock(&sync->network_mutex);
                break;
            case 1:
                pthread_mutex_lock(&sync->weight_mutex);
                usleep(1);
                pthread_mutex_unlock(&sync->weight_mutex);
                break;
            case 2:
                pthread_mutex_lock(&sync->data_mutex);
                usleep(1);
                pthread_mutex_unlock(&sync->data_mutex);
                break;
            case 3:
                pthread_mutex_lock(&sync->gpu_mutex);
                usleep(1);
                pthread_mutex_unlock(&sync->gpu_mutex);
                break;
            case 4:
                pthread_mutex_lock(&sync->stats_mutex);
                atomic_fetch_add(&g_counter, 1);
                pthread_mutex_unlock(&sync->stats_mutex);
                break;
        }
    }
    
    return NULL;
}

void test_stress_high_contention() {
    printf("Testing high contention scenarios...\n");
    
    sync_mutexes sync;
    init_sync_mutexes(&sync);
    
    pthread_t threads[NUM_THREADS * 2];  // Double the threads for stress
    thread_test_args args[NUM_THREADS * 2];
    
    atomic_store(&g_counter, 0);
    
    for (int i = 0; i < NUM_THREADS * 2; i++) {
        args[i].thread_id = i;
        args[i].iterations = 100;  // Fewer iterations due to sleep
        args[i].shared_data = &sync;
        pthread_create(&threads[i], NULL, stress_test_thread, &args[i]);
    }
    
    for (int i = 0; i < NUM_THREADS * 2; i++) {
        pthread_join(threads[i], NULL);
    }
    
    destroy_sync_mutexes(&sync);
    printf("✓ High contention stress test passed\n");
}

int main(int argc, char **argv) {
    printf("\n=== Thread Safety Test Suite ===\n\n");
    
    test_basic_mutex();
    test_network_sync_context();
    test_demo_sync_context();
    test_concurrent_layer_updates();
    test_memory_barriers();
    test_race_condition_prevention();
    test_stress_high_contention();
    
    printf("\n=== All Thread Safety Tests Passed ===\n\n");
    
    return 0;
}