#include "config.h"
#include "esp_camera.h"

// Are we recording?
int IS_RECORDING = false;

// ID to prevent collision in file name.
int TAKE_ID = 0;

// How many frames have we captured?
int CAPTURED_FRAME_COUNT = 0;

// How many frames have we processed?
int PROCESSED_FRAME_COUNT = 0;

// Task handles.
TaskHandle_t CameraTask;
TaskHandle_t SaveFrameTask;

// Queue handle.
QueueHandle_t framebuffer_queue;