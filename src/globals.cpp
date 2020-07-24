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

// The shared list of frame-buffers.
camera_fb_t *framebuffer_queue[FRAME_BUFFER_QUEUE_SIZE];
