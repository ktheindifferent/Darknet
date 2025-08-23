#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include "../include/thread_sync.h"

#define NUM_THREADS 8
#define NUM_ITERATIONS 1000000
#define WARMUP_ITERATIONS 10000

// Performance measurement utilities
double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

// Test structures
typedef struct {
    int thread_id;
    int iterations;
    pthread_mutex_t *mutex;
    atomic_int *atomic_counter;
    int *counter;
    double *thread_time;
} perf_test_args;

// Baseline: No synchronization (unsafe, for comparison only)
void *baseline_thread(void *arg) {
    perf_test_args *args = (perf_test_args*)arg;
    double start = get_time();
    
    for (int i = 0; i < args->iterations; i++) {
        (*args->counter)++;  // Unsafe increment
    }
    
    args->thread_time[args->thread_id] = get_time() - start;
    return NULL;
}

// Test 1: Mutex synchronization
void *mutex_thread(void *arg) {
    perf_test_args *args = (perf_test_args*)arg;
    double start = get_time();
    
    for (int i = 0; i < args->iterations; i++) {
        pthread_mutex_lock(args->mutex);
        (*args->counter)++;
        pthread_mutex_unlock(args->mutex);
    }
    
    args->thread_time[args->thread_id] = get_time() - start;
    return NULL;
}

// Test 2: Atomic operations
void *atomic_thread(void *arg) {
    perf_test_args *args = (perf_test_args*)arg;
    double start = get_time();
    
    for (int i = 0; i < args->iterations; i++) {
        atomic_fetch_add(args->atomic_counter, 1);
    }
    
    args->thread_time[args->thread_id] = get_time() - start;
    return NULL;
}

// Test 3: Fine-grained locking (multiple mutexes)
void *fine_grained_thread(void *arg) {
    perf_test_args *args = (perf_test_args*)arg;
    network_sync_context *ctx = create_network_sync_context(16);
    double start = get_time();
    
    for (int i = 0; i < args->iterations; i++) {
        int layer = i % 16;
        pthread_mutex_lock(&ctx->layer_mutexes[layer]);
        (*args->counter)++;
        pthread_mutex_unlock(&ctx->layer_mutexes[layer]);
    }
    
    args->thread_time[args->thread_id] = get_time() - start;
    destroy_network_sync_context(ctx);
    return NULL;
}

// Run performance test
double run_perf_test(void *(*thread_func)(void*), int num_threads, int iterations, 
                     const char *test_name) {
    pthread_t threads[num_threads];
    perf_test_args args[num_threads];
    double thread_times[num_threads];
    
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
    
    atomic_int atomic_counter = 0;
    int counter = 0;
    
    // Warmup
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        counter++;
    }
    counter = 0;
    
    printf("\n%s (%d threads, %d iterations each):\n", test_name, num_threads, iterations);
    
    double start_time = get_time();
    
    for (int i = 0; i < num_threads; i++) {
        args[i].thread_id = i;
        args[i].iterations = iterations;
        args[i].mutex = &mutex;
        args[i].atomic_counter = &atomic_counter;
        args[i].counter = &counter;
        args[i].thread_time = thread_times;
        pthread_create(&threads[i], NULL, thread_func, &args[i]);
    }
    
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    double total_time = get_time() - start_time;
    
    // Calculate statistics
    double min_time = thread_times[0];
    double max_time = thread_times[0];
    double avg_time = thread_times[0];
    
    for (int i = 1; i < num_threads; i++) {
        if (thread_times[i] < min_time) min_time = thread_times[i];
        if (thread_times[i] > max_time) max_time = thread_times[i];
        avg_time += thread_times[i];
    }
    avg_time /= num_threads;
    
    printf("  Total time: %.4f seconds\n", total_time);
    printf("  Avg thread time: %.4f seconds\n", avg_time);
    printf("  Min/Max thread time: %.4f / %.4f seconds\n", min_time, max_time);
    printf("  Operations per second: %.2f million\n", 
           (num_threads * iterations) / (total_time * 1e6));
    
    pthread_mutex_destroy(&mutex);
    
    return total_time;
}

// Contention test
void test_contention_scaling() {
    printf("\n=== Contention Scaling Test ===\n");
    printf("Testing how performance scales with thread count...\n");
    
    int thread_counts[] = {1, 2, 4, 8, 16};
    int num_tests = sizeof(thread_counts) / sizeof(thread_counts[0]);
    
    for (int i = 0; i < num_tests; i++) {
        int threads = thread_counts[i];
        int iterations_per_thread = NUM_ITERATIONS / threads;
        
        printf("\n--- %d Threads ---\n", threads);
        
        // Test mutex
        double mutex_time = run_perf_test(mutex_thread, threads, iterations_per_thread, 
                                          "Mutex Protection");
        
        // Test atomic
        double atomic_time = run_perf_test(atomic_thread, threads, iterations_per_thread,
                                           "Atomic Operations");
        
        printf("\nSpeedup (Atomic vs Mutex): %.2fx\n", mutex_time / atomic_time);
    }
}

// Memory barrier overhead test
void test_memory_barrier_overhead() {
    printf("\n=== Memory Barrier Overhead Test ===\n");
    
    const int iterations = 10000000;
    double start, end;
    
    // Test without barriers
    start = get_time();
    for (int i = 0; i < iterations; i++) {
        // Just a simple operation
        volatile int x = i;
        (void)x;
    }
    end = get_time();
    double no_barrier_time = end - start;
    printf("No barriers: %.6f seconds\n", no_barrier_time);
    
    // Test with acquire fence
    start = get_time();
    for (int i = 0; i < iterations; i++) {
        acquire_fence();
        volatile int x = i;
        (void)x;
    }
    end = get_time();
    double acquire_time = end - start;
    printf("With acquire fence: %.6f seconds (%.2f%% overhead)\n", 
           acquire_time, ((acquire_time - no_barrier_time) / no_barrier_time) * 100);
    
    // Test with release fence
    start = get_time();
    for (int i = 0; i < iterations; i++) {
        volatile int x = i;
        (void)x;
        release_fence();
    }
    end = get_time();
    double release_time = end - start;
    printf("With release fence: %.6f seconds (%.2f%% overhead)\n",
           release_time, ((release_time - no_barrier_time) / no_barrier_time) * 100);
    
    // Test with full memory barrier
    start = get_time();
    for (int i = 0; i < iterations; i++) {
        memory_barrier();
        volatile int x = i;
        (void)x;
    }
    end = get_time();
    double full_barrier_time = end - start;
    printf("With full memory barrier: %.6f seconds (%.2f%% overhead)\n",
           full_barrier_time, ((full_barrier_time - no_barrier_time) / no_barrier_time) * 100);
}

// Context creation/destruction overhead
void test_context_overhead() {
    printf("\n=== Context Creation/Destruction Overhead ===\n");
    
    const int iterations = 10000;
    double start, end;
    
    // Test network_sync_context
    start = get_time();
    for (int i = 0; i < iterations; i++) {
        network_sync_context *ctx = create_network_sync_context(32);
        destroy_network_sync_context(ctx);
    }
    end = get_time();
    printf("Network sync context: %.6f seconds for %d create/destroy cycles\n",
           end - start, iterations);
    printf("  Average: %.3f microseconds per cycle\n", 
           ((end - start) / iterations) * 1e6);
    
    // Test demo_sync_context
    start = get_time();
    for (int i = 0; i < iterations; i++) {
        demo_sync_context *ctx = create_demo_sync_context();
        destroy_demo_sync_context(ctx);
    }
    end = get_time();
    printf("Demo sync context: %.6f seconds for %d create/destroy cycles\n",
           end - start, iterations);
    printf("  Average: %.3f microseconds per cycle\n",
           ((end - start) / iterations) * 1e6);
}

int main(int argc, char **argv) {
    printf("\n=== Thread Synchronization Performance Tests ===\n");
    printf("Testing overhead and scalability of synchronization primitives\n");
    
    // Basic performance comparison
    printf("\n=== Basic Performance Comparison ===\n");
    run_perf_test(baseline_thread, NUM_THREADS, NUM_ITERATIONS/NUM_THREADS,
                  "Baseline (No Sync - UNSAFE)");
    run_perf_test(mutex_thread, NUM_THREADS, NUM_ITERATIONS/NUM_THREADS,
                  "Mutex Protection");
    run_perf_test(atomic_thread, NUM_THREADS, NUM_ITERATIONS/NUM_THREADS,
                  "Atomic Operations");
    run_perf_test(fine_grained_thread, NUM_THREADS, NUM_ITERATIONS/NUM_THREADS,
                  "Fine-grained Locking (16 mutexes)");
    
    // Contention scaling
    test_contention_scaling();
    
    // Memory barrier overhead
    test_memory_barrier_overhead();
    
    // Context overhead
    test_context_overhead();
    
    printf("\n=== Performance Testing Complete ===\n\n");
    
    return 0;
}