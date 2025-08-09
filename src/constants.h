#ifndef CONSTANTS_H
#define CONSTANTS_H

/**
 * Common constants used throughout the Darknet codebase
 * Extracted to improve code maintainability and clarity
 */

// Buffer sizes
#define BUFFER_SIZE_SMALL   256
#define BUFFER_SIZE_LARGE   4096
#define PATH_BUFFER_SIZE    256
#define STRING_BUFFER_SIZE  4096

// Workspace limits
#define MAX_WORKSPACE_SIZE  2000000000  // 2GB limit for workspace

// Image processing constants
#define DEFAULT_CHANNELS    3
#define MAX_PIXEL_VALUE     255
#define MIN_PIXEL_VALUE     0

// Default augmentation values
#define DEFAULT_EXPOSURE    1.0f
#define DEFAULT_SATURATION  1.0f
#define DEFAULT_ASPECT      1.0f

// Network constants
#define DEFAULT_BATCH       1
#define DEFAULT_LEARNING_RATE 0.001f

// Random seed (can be made configurable)
#define DEFAULT_RANDOM_SEED 2222222

// File I/O constants
#define MAX_LINE_LENGTH     4096
#define DEFAULT_FILE_MODE   "r"

// Detection thresholds
#define DEFAULT_NMS_THRESHOLD 0.45f
#define DEFAULT_DETECTION_THRESHOLD 0.5f

// Memory alignment
#define MEMORY_ALIGNMENT    32

#endif // CONSTANTS_H