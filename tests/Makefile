CC=gcc
CFLAGS=-Wall -Wno-unused-result -Wno-unknown-pragmas -Wfatal-errors -fPIC -Ofast -I../include -I../src
LDFLAGS=-lm -pthread
OBJDIR=../obj/

# Object files from main project needed for tests
UTILS_OBJS=$(OBJDIR)utils.o $(OBJDIR)list.o
DATA_OBJS=$(OBJDIR)data.o $(OBJDIR)utils.o $(OBJDIR)list.o $(OBJDIR)matrix.o $(OBJDIR)image.o $(OBJDIR)box.o
NETWORK_OBJS=$(OBJDIR)network.o $(OBJDIR)layer.o $(OBJDIR)utils.o $(OBJDIR)blas.o $(OBJDIR)cost_layer.o \
             $(OBJDIR)convolutional_layer.o $(OBJDIR)activation_layer.o $(OBJDIR)activations.o \
             $(OBJDIR)maxpool_layer.o $(OBJDIR)softmax_layer.o $(OBJDIR)dropout_layer.o \
             $(OBJDIR)normalization_layer.o $(OBJDIR)batchnorm_layer.o $(OBJDIR)connected_layer.o \
             $(OBJDIR)avgpool_layer.o $(OBJDIR)local_layer.o $(OBJDIR)shortcut_layer.o \
             $(OBJDIR)route_layer.o $(OBJDIR)upsample_layer.o $(OBJDIR)yolo_layer.o \
             $(OBJDIR)region_layer.o $(OBJDIR)reorg_layer.o $(OBJDIR)detection_layer.o \
             $(OBJDIR)logistic_layer.o $(OBJDIR)l2norm_layer.o $(OBJDIR)rnn_layer.o \
             $(OBJDIR)gru_layer.o $(OBJDIR)lstm_layer.o $(OBJDIR)crnn_layer.o $(OBJDIR)iseg_layer.o \
             $(OBJDIR)gemm.o $(OBJDIR)im2col.o $(OBJDIR)col2im.o $(OBJDIR)crop_layer.o
BOX_OBJS=$(OBJDIR)box.o
IMAGE_OBJS=$(OBJDIR)image.o $(OBJDIR)utils.o $(OBJDIR)blas.o $(OBJDIR)list.o
BLAS_OBJS=$(OBJDIR)blas.o

# Test executables
TESTS=test_utils test_data test_network test_box test_image test_blas test_memory

# Ensure we have obj directory
$(shell mkdir -p $(OBJDIR))

all: $(TESTS)

test_utils: test_utils.c $(UTILS_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test_data: test_data.c $(DATA_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test_network: test_network.c $(NETWORK_OBJS) $(DATA_OBJS) $(BOX_OBJS) $(IMAGE_OBJS) $(OBJDIR)parser.o $(OBJDIR)option_list.o $(OBJDIR)tree.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test_box: test_box.c $(BOX_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test_image: test_image.c $(IMAGE_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test_blas: test_blas.c $(BLAS_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test_memory: test_memory.c $(UTILS_OBJS) $(OBJDIR)matrix.o $(OBJDIR)option_list.o $(OBJDIR)blas.o $(OBJDIR)tree.o $(DATA_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Run all tests
test: $(TESTS)
	@echo "\n===== Running All Tests ====="
	@for test in $(TESTS); do \
		echo "\nRunning $$test..."; \
		./$$test || exit 1; \
	done
	@echo "\n===== All Tests Completed Successfully ====="

# Run individual tests
run_%: %
	./$<

# Clean test executables
clean:
	rm -f $(TESTS)
	rm -f /tmp/test_*.txt

# Build main project objects if needed
$(OBJDIR)%.o: ../src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all test clean