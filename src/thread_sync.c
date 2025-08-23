#include "thread_sync.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void init_sync_mutexes(sync_mutexes *sync) {
    if (!sync) return;
    
    pthread_mutex_init(&sync->network_mutex, NULL);
    pthread_mutex_init(&sync->weight_mutex, NULL);
    pthread_mutex_init(&sync->data_mutex, NULL);
    pthread_mutex_init(&sync->gpu_mutex, NULL);
    pthread_mutex_init(&sync->stats_mutex, NULL);
}

void destroy_sync_mutexes(sync_mutexes *sync) {
    if (!sync) return;
    
    pthread_mutex_destroy(&sync->network_mutex);
    pthread_mutex_destroy(&sync->weight_mutex);
    pthread_mutex_destroy(&sync->data_mutex);
    pthread_mutex_destroy(&sync->gpu_mutex);
    pthread_mutex_destroy(&sync->stats_mutex);
}

network_sync_context* create_network_sync_context(int num_layers) {
    network_sync_context *ctx = calloc(1, sizeof(network_sync_context));
    if (!ctx) return NULL;
    
    ctx->num_layers = num_layers;
    ctx->layer_mutexes = calloc(num_layers, sizeof(pthread_mutex_t));
    if (!ctx->layer_mutexes) {
        free(ctx);
        return NULL;
    }
    
    for (int i = 0; i < num_layers; i++) {
        pthread_mutex_init(&ctx->layer_mutexes[i], NULL);
    }
    
    atomic_init(&ctx->active_threads, 0);
    atomic_init(&ctx->seen_samples, 0);
    
    return ctx;
}

void destroy_network_sync_context(network_sync_context *ctx) {
    if (!ctx) return;
    
    if (ctx->layer_mutexes) {
        for (int i = 0; i < ctx->num_layers; i++) {
            pthread_mutex_destroy(&ctx->layer_mutexes[i]);
        }
        free(ctx->layer_mutexes);
    }
    
    free(ctx);
}

demo_sync_context* create_demo_sync_context(void) {
    demo_sync_context *ctx = calloc(1, sizeof(demo_sync_context));
    if (!ctx) return NULL;
    
    pthread_mutex_init(&ctx->buffer_mutex, NULL);
    pthread_mutex_init(&ctx->prediction_mutex, NULL);
    pthread_mutex_init(&ctx->display_mutex, NULL);
    pthread_cond_init(&ctx->buffer_ready, NULL);
    pthread_cond_init(&ctx->detection_done, NULL);
    
    atomic_init(&ctx->buffer_index, 0);
    atomic_init(&ctx->demo_running, 1);
    
    return ctx;
}

void destroy_demo_sync_context(demo_sync_context *ctx) {
    if (!ctx) return;
    
    pthread_mutex_destroy(&ctx->buffer_mutex);
    pthread_mutex_destroy(&ctx->prediction_mutex);
    pthread_mutex_destroy(&ctx->display_mutex);
    pthread_cond_destroy(&ctx->buffer_ready);
    pthread_cond_destroy(&ctx->detection_done);
    
    free(ctx);
}

void sync_network_update(network_sync_context *ctx, int layer_idx, 
                         void (*update_func)(void*), void *data) {
    if (!ctx || !update_func || layer_idx < 0 || layer_idx >= ctx->num_layers) {
        return;
    }
    
    pthread_mutex_lock(&ctx->layer_mutexes[layer_idx]);
    update_func(data);
    pthread_mutex_unlock(&ctx->layer_mutexes[layer_idx]);
}

void sync_weight_accumulate(sync_mutexes *sync, 
                            void (*accumulate_func)(void*), void *data) {
    if (!sync || !accumulate_func) return;
    
    pthread_mutex_lock(&sync->weight_mutex);
    accumulate_func(data);
    pthread_mutex_unlock(&sync->weight_mutex);
}