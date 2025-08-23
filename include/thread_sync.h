#ifndef THREAD_SYNC_H
#define THREAD_SYNC_H

#include <pthread.h>
#include <stdatomic.h>

// Global mutexes for different subsystems
typedef struct {
    pthread_mutex_t network_mutex;      // Protects network structure modifications
    pthread_mutex_t weight_mutex;        // Protects weight updates
    pthread_mutex_t data_mutex;          // Protects shared data access
    pthread_mutex_t gpu_mutex;           // Protects GPU operations
    pthread_mutex_t stats_mutex;         // Protects statistics updates
} sync_mutexes;

// Thread-safe network training context
typedef struct {
    pthread_mutex_t *layer_mutexes;     // One mutex per layer
    int num_layers;
    atomic_int active_threads;
    atomic_long seen_samples;           // Atomic counter for samples seen
} network_sync_context;

// Demo-specific synchronization
typedef struct {
    pthread_mutex_t buffer_mutex;        // Protects image buffer access
    pthread_mutex_t prediction_mutex;    // Protects prediction arrays
    pthread_mutex_t display_mutex;       // Protects display operations
    pthread_cond_t buffer_ready;         // Condition variable for buffer readiness
    pthread_cond_t detection_done;       // Condition variable for detection completion
    atomic_int buffer_index;             // Atomic buffer index
    atomic_bool demo_running;            // Atomic flag for demo state
} demo_sync_context;

// Thread barrier for synchronization points
typedef struct {
    pthread_barrier_t barrier;
    int thread_count;
} sync_barrier;

// Initialize synchronization contexts
void init_sync_mutexes(sync_mutexes *sync);
void destroy_sync_mutexes(sync_mutexes *sync);

network_sync_context* create_network_sync_context(int num_layers);
void destroy_network_sync_context(network_sync_context *ctx);

demo_sync_context* create_demo_sync_context(void);
void destroy_demo_sync_context(demo_sync_context *ctx);

// Thread-safe operations
void sync_network_update(network_sync_context *ctx, int layer_idx, 
                         void (*update_func)(void*), void *data);
void sync_weight_accumulate(sync_mutexes *sync, 
                            void (*accumulate_func)(void*), void *data);

// Memory barrier operations
static inline void memory_barrier(void) {
    __sync_synchronize();
}

static inline void acquire_fence(void) {
    atomic_thread_fence(memory_order_acquire);
}

static inline void release_fence(void) {
    atomic_thread_fence(memory_order_release);
}

#endif // THREAD_SYNC_H