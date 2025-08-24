#include "normalization_layer.h"
#include "blas.h"

#include <stdio.h>
#include "utils.h"

layer make_normalization_layer(int batch, int w, int h, int c, int size, float alpha, float beta, float kappa)
{
    fprintf(stderr, "Local Response Normalization Layer: %d x %d x %d image, %d size\n", w,h,c,size);
    layer layer = {0};
    layer.type = NORMALIZATION;
    layer.batch = batch;
    layer.h = layer.out_h = h;
    layer.w = layer.out_w = w;
    layer.c = layer.out_c = c;
    layer.kappa = kappa;
    layer.size = size;
    layer.alpha = alpha;
    layer.beta = beta;
    layer.output = safe_calloc(h * w * c * batch, sizeof(float));
    layer.delta = safe_calloc(h * w * c * batch, sizeof(float));
    layer.squared = safe_calloc(h * w * c * batch, sizeof(float));
    layer.norms = safe_calloc(h * w * c * batch, sizeof(float));
    layer.inputs = w*h*c;
    layer.outputs = layer.inputs;

    layer.forward = forward_normalization_layer;
    layer.backward = backward_normalization_layer;
    #ifdef GPU
    layer.forward_gpu = forward_normalization_layer_gpu;
    layer.backward_gpu = backward_normalization_layer_gpu;

    layer.output_gpu =  cuda_make_array(layer.output, h * w * c * batch);
    layer.delta_gpu =   cuda_make_array(layer.delta, h * w * c * batch);
    layer.squared_gpu = cuda_make_array(layer.squared, h * w * c * batch);
    layer.norms_gpu =   cuda_make_array(layer.norms, h * w * c * batch);
    #endif
    return layer;
}

void resize_normalization_layer(layer *layer, int w, int h)
{
    int c = layer->c;
    int batch = layer->batch;
    layer->h = h;
    layer->w = w;
    layer->out_h = h;
    layer->out_w = w;
    layer->inputs = w*h*c;
    layer->outputs = layer->inputs;
    layer->output = safe_realloc(layer->output, h * w * c * batch * sizeof(float));
    layer->delta = safe_realloc(layer->delta, h * w * c * batch * sizeof(float));
    layer->squared = safe_realloc(layer->squared, h * w * c * batch * sizeof(float));
    layer->norms = safe_realloc(layer->norms, h * w * c * batch * sizeof(float));
#ifdef GPU
    cuda_free(layer->output_gpu);
    cuda_free(layer->delta_gpu); 
    cuda_free(layer->squared_gpu); 
    cuda_free(layer->norms_gpu);   
    layer->output_gpu =  cuda_make_array(layer->output, h * w * c * batch);
    layer->delta_gpu =   cuda_make_array(layer->delta, h * w * c * batch);
    layer->squared_gpu = cuda_make_array(layer->squared, h * w * c * batch);
    layer->norms_gpu =   cuda_make_array(layer->norms, h * w * c * batch);
#endif
}

void forward_normalization_layer(const layer layer, network net)
{
    int k,b;
    int w = layer.w;
    int h = layer.h;
    int c = layer.c;
    scal_cpu(w*h*c*layer.batch, 0, layer.squared, 1);

    for(b = 0; b < layer.batch; ++b){
        float *squared = layer.squared + w*h*c*b;
        float *norms   = layer.norms + w*h*c*b;
        float *input   = net.input + w*h*c*b;
        pow_cpu(w*h*c, 2, input, 1, squared, 1);

        const_cpu(w*h, layer.kappa, norms, 1);
        for(k = 0; k < layer.size/2; ++k){
            axpy_cpu(w*h, layer.alpha, squared + w*h*k, 1, norms, 1);
        }

        for(k = 1; k < layer.c; ++k){
            copy_cpu(w*h, norms + w*h*(k-1), 1, norms + w*h*k, 1);
            int prev = k - ((layer.size-1)/2) - 1;
            int next = k + (layer.size/2);
            if(prev >= 0)      axpy_cpu(w*h, -layer.alpha, squared + w*h*prev, 1, norms + w*h*k, 1);
            if(next < layer.c) axpy_cpu(w*h,  layer.alpha, squared + w*h*next, 1, norms + w*h*k, 1);
        }
    }
    pow_cpu(w*h*c*layer.batch, -layer.beta, layer.norms, 1, layer.output, 1);
    mul_cpu(w*h*c*layer.batch, net.input, 1, layer.output, 1);
}

void backward_normalization_layer(const layer layer, network net)
{
    // Correct implementation of Local Response Normalization backward pass
    // Based on the forward pass: output[i] = input[i] / (norms[i])^beta
    // where norms[i] = kappa + alpha * sum_j(input[j]^2) for j in window
    
    int b, k, i;
    int w = layer.w;
    int h = layer.h;
    int c = layer.c;
    int spatial = w * h;
    
    for(b = 0; b < layer.batch; ++b){
        float *input = net.input + spatial*c*b;
        float *output = layer.output + spatial*c*b;
        float *delta_in = net.delta + spatial*c*b;
        float *delta_out = layer.delta + spatial*c*b;
        float *norms = layer.norms + spatial*c*b;
        
        // For each channel
        for(k = 0; k < c; ++k){
            // Compute window range for channel k
            int min_c = k - layer.size/2;
            int max_c = k + layer.size/2;
            if(min_c < 0) min_c = 0;
            if(max_c >= c) max_c = c - 1;
            
            for(int s = 0; s < spatial; ++s){
                int idx_k = k * spatial + s;
                
                // Gradient has two components:
                // 1. Direct path where input[k] affects output[k]
                // 2. Indirect paths where input[k] affects output[i] for i in neighborhood
                
                // Direct component
                float grad = delta_out[idx_k] * pow(norms[idx_k], -layer.beta);
                
                // Indirect components - input[k] affects all outputs in its window
                float factor = -2.0f * layer.alpha * layer.beta * input[idx_k];
                for(i = min_c; i <= max_c; ++i){
                    int idx_i = i * spatial + s;
                    // Gradient from output[i] through normalization
                    grad += delta_out[idx_i] * output[idx_i] * factor * pow(norms[idx_i], -1.0f);
                }
                
                // Add to existing gradient
                delta_in[idx_k] += grad;
            }
        }
    }
}

#ifdef GPU
void forward_normalization_layer_gpu(const layer layer, network net)
{
    int k,b;
    int w = layer.w;
    int h = layer.h;
    int c = layer.c;
    scal_gpu(w*h*c*layer.batch, 0, layer.squared_gpu, 1);

    for(b = 0; b < layer.batch; ++b){
        float *squared = layer.squared_gpu + w*h*c*b;
        float *norms   = layer.norms_gpu + w*h*c*b;
        float *input   = net.input_gpu + w*h*c*b;
        pow_gpu(w*h*c, 2, input, 1, squared, 1);

        const_gpu(w*h, layer.kappa, norms, 1);
        for(k = 0; k < layer.size/2; ++k){
            axpy_gpu(w*h, layer.alpha, squared + w*h*k, 1, norms, 1);
        }

        for(k = 1; k < layer.c; ++k){
            copy_gpu(w*h, norms + w*h*(k-1), 1, norms + w*h*k, 1);
            int prev = k - ((layer.size-1)/2) - 1;
            int next = k + (layer.size/2);
            if(prev >= 0)      axpy_gpu(w*h, -layer.alpha, squared + w*h*prev, 1, norms + w*h*k, 1);
            if(next < layer.c) axpy_gpu(w*h,  layer.alpha, squared + w*h*next, 1, norms + w*h*k, 1);
        }
    }
    pow_gpu(w*h*c*layer.batch, -layer.beta, layer.norms_gpu, 1, layer.output_gpu, 1);
    mul_gpu(w*h*c*layer.batch, net.input_gpu, 1, layer.output_gpu, 1);
}

void backward_normalization_layer_gpu(const layer layer, network net)
{
    // GPU implementation of Local Response Normalization backward pass
    // Uses CPU computation for complex indexing, then pushes back to GPU
    
    int b, k, i;
    int w = layer.w;
    int h = layer.h;
    int c = layer.c;
    int spatial = w * h;
    int total_size = spatial * c * layer.batch;
    
    // Copy data from GPU to CPU for computation
    float *input_cpu = calloc(total_size, sizeof(float));
    float *output_cpu = calloc(total_size, sizeof(float));
    float *delta_in_cpu = calloc(total_size, sizeof(float));
    float *delta_out_cpu = calloc(total_size, sizeof(float));
    float *norms_cpu = calloc(total_size, sizeof(float));
    
    cuda_pull_array(net.input_gpu, input_cpu, total_size);
    cuda_pull_array(layer.output_gpu, output_cpu, total_size);
    cuda_pull_array(net.delta_gpu, delta_in_cpu, total_size);
    cuda_pull_array(layer.delta_gpu, delta_out_cpu, total_size);
    cuda_pull_array(layer.norms_gpu, norms_cpu, total_size);
    
    for(b = 0; b < layer.batch; ++b){
        float *input = input_cpu + spatial*c*b;
        float *output = output_cpu + spatial*c*b;
        float *delta_in = delta_in_cpu + spatial*c*b;
        float *delta_out = delta_out_cpu + spatial*c*b;
        float *norms = norms_cpu + spatial*c*b;
        
        // For each channel
        for(k = 0; k < c; ++k){
            // Compute window range for channel k
            int min_c = k - layer.size/2;
            int max_c = k + layer.size/2;
            if(min_c < 0) min_c = 0;
            if(max_c >= c) max_c = c - 1;
            
            for(int s = 0; s < spatial; ++s){
                int idx_k = k * spatial + s;
                
                // Direct component
                float grad = delta_out[idx_k] * pow(norms[idx_k], -layer.beta);
                
                // Indirect components
                float factor = -2.0f * layer.alpha * layer.beta * input[idx_k];
                for(i = min_c; i <= max_c; ++i){
                    int idx_i = i * spatial + s;
                    grad += delta_out[idx_i] * output[idx_i] * factor * pow(norms[idx_i], -1.0f);
                }
                
                // Add to existing gradient
                delta_in[idx_k] += grad;
            }
        }
    }
    
    // Push the result back to GPU
    cuda_push_array(net.delta_gpu, delta_in_cpu, total_size);
    
    // Free temporary CPU memory
    free(input_cpu);
    free(output_cpu);
    free(delta_in_cpu);
    free(delta_out_cpu);
    free(norms_cpu);
}
#endif
