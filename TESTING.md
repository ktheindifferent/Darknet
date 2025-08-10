# Darknet Testing Documentation

## Overview
This document describes the testing infrastructure created for the Darknet neural network framework. The test suite covers core functionality including utility functions, data processing, network operations, and mathematical operations.

## Test Structure

### Test Files
The testing infrastructure is located in the `tests/` directory:

```
tests/
├── test_utils.c       # Utility function tests
├── test_data.c        # Data loading and processing tests
├── test_network.c     # Neural network operation tests
├── test_box.c         # Bounding box calculation tests
├── test_image.c       # Image processing tests
├── test_blas.c        # BLAS operation tests
├── test_simplified.c  # Public API tests
├── Makefile          # Build system for tests
├── Makefile.simple   # Simplified build for public API
└── run_tests.sh      # Automated test runner
```

## Building and Running Tests

### Prerequisites
1. Build the main Darknet project first:
```bash
make clean
make
```

### Running All Tests
Execute the test runner script:
```bash
cd tests
./run_tests.sh
```

### Running Individual Test Modules
```bash
cd tests
make test_utils
./test_utils
```

### Running Simplified Public API Tests
```bash
cd tests
make -f Makefile.simple test
```

## Test Coverage

### 1. Utility Functions (test_utils.c)
- Command-line argument parsing
- String manipulation (strip, find_replace)
- File I/O operations
- List operations
- Integer list parsing

### 2. Data Operations (test_data.c)
- Matrix creation and manipulation
- Data batch allocation
- Label loading
- Truth value filling

### 3. Network Operations (test_network.c)
- Network creation and destruction
- Learning rate calculations
- Network output retrieval
- Accuracy calculations
- Top-k selection

### 4. Box Operations (test_box.c)
- IoU (Intersection over Union) calculations
- RMSE (Root Mean Square Error)
- NMS (Non-Maximum Suppression)
- Box encoding/decoding
- YOLO box transformations

### 5. Image Operations (test_image.c)
- Image creation and destruction
- Image resizing and letterboxing
- Pixel get/set operations
- Color space conversions (RGB to HSV)
- Image flipping and transformations
- Grayscale conversion
- Thresholding

### 6. BLAS Operations (test_blas.c)
- Vector operations (fill, copy, axpy, scal)
- Dot product calculations
- Mean and variance calculations
- Normalization
- Softmax
- Bias operations
- L2 norm calculations

## Test Methodology

### Unit Testing Approach
- Each test function is independent
- Tests use assertions to verify expected behavior
- Clear success/failure reporting
- Minimal external dependencies

### Test Data
- Tests use synthetic data when possible
- Temporary files are created in `/tmp` for file I/O tests
- All temporary resources are cleaned up after tests

### Error Handling
- Tests check for both success and failure cases
- Memory allocation is verified
- File operations include error checking

## Known Limitations

### API Boundaries
Some functions tested are internal implementations not exposed in the public API:
- `normalize_load_args` (static function)
- `get_random_paths` (internal utility)
- `find_replace_paths` (internal utility)
- `resize_layer_by_type` (internal function)
- `box_intersection` / `box_union` (internal, used by box_iou)

### GPU Testing
Current tests focus on CPU implementations. GPU functionality requires:
- CUDA installation
- GPU hardware
- Separate GPU-specific test suite

### Integration Testing
Current tests are primarily unit tests. Integration testing would require:
- Pre-trained models
- Test datasets
- Performance benchmarks

## Future Improvements

### High Priority
1. Add integration tests with actual models
2. Create performance benchmarks
3. Add memory leak detection (Valgrind integration)
4. Implement code coverage reporting

### Medium Priority
1. Add GPU functionality tests
2. Create stress tests for large data
3. Add multi-threaded operation tests
4. Implement continuous integration

### Low Priority
1. Add fuzzing tests for input validation
2. Create visual regression tests for image operations
3. Add compatibility tests across platforms

## Contributing

When adding new functionality:
1. Write corresponding unit tests
2. Ensure tests pass before committing
3. Update this documentation
4. Follow existing test patterns

### Test Writing Guidelines
```c
void test_function_name() {
    printf("Testing function_name...\n");
    
    // Setup test data
    // Perform operations
    // Assert expected results
    
    printf("  ✓ function_name tests passed\n");
}
```

## Continuous Integration

For CI/CD integration, use:
```bash
cd tests
./run_tests.sh
exit_code=$?
exit $exit_code
```

The script returns:
- 0: All tests passed
- 1: One or more tests failed

## Troubleshooting

### Common Issues

1. **Linking Errors**: Ensure main project is built first
2. **Missing Dependencies**: Check that all object files are compiled
3. **Permission Errors**: Make test scripts executable with `chmod +x`
4. **Path Issues**: Run tests from the `tests/` directory

### Debug Mode
To build tests with debug symbols:
```bash
make clean
make CFLAGS="-g -O0"
```

Then use GDB for debugging:
```bash
gdb ./test_utils
```

## Maintenance

### Regular Tasks
1. Run tests before major commits
2. Update tests when API changes
3. Review and refactor test code periodically
4. Monitor test execution time

### Test Metrics
Track:
- Number of tests
- Code coverage percentage
- Test execution time
- Test failure rate

## Contact

For test-related issues:
1. Check this documentation first
2. Review existing test code
3. Consult the main README.md
4. Open an issue on the project repository