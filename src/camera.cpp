#include "camera.h"

#include <Arduino.h>
#include <EEPROM.h>
#include <SD_MMC.h>

#include "config.h"
#include "esp_camera.h"
#include "globals.h"
#include "notify_error.h"

camera_config_t create_camera_config() {
  camera_config_t config;

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    notify_warning();

    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  return config;
}

esp_err_t setup_camera() {
  camera_config_t camera_config = create_camera_config();

  esp_err_t err = esp_camera_init(&camera_config);

  if (err != ESP_OK) {
    if (ENABLE_SERIAL_LOG) Serial.printf("Camera init failed with error 0x%x", err);

    return ESP_FAIL;
  }

  if (!SD_MMC.begin("/sdcard")) {
    if (ENABLE_SERIAL_LOG) Serial.println("SD Card Mount Failed");

    return ESP_FAIL;
  }

  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    if (ENABLE_SERIAL_LOG) Serial.println("No SD Card attached");

    return ESP_FAIL;
  }

  sensor_t *sensor = esp_camera_sensor_get();

  // Lower the saturation.
  sensor->set_saturation(sensor, -2);
  sensor->set_brightness(sensor, 0);

  return ESP_OK;
}

// Capture the frame.
esp_err_t capture_frame() {
  camera_fb_t *frame_buffer = esp_camera_fb_get();

  if (!frame_buffer) {
    if (ENABLE_SERIAL_LOG) Serial.println("Camera capture failed");

    return ESP_FAIL;
  }

  xQueueSend(framebuffer_queue, &frame_buffer, portMAX_DELAY);
  CAPTURED_FRAME_COUNT += 1;

  return ESP_OK;
}

// Save the frame to disk.
esp_err_t save_frame() {
  camera_fb_t *frame_buffer = NULL;

  // Path where new picture will be saved in SD Card
  String file_name = "take_" + String(TAKE_ID) + "_frame_" + String(PROCESSED_FRAME_COUNT);
  String path = "/" + file_name + ".jpg";

  if (xQueueReceive(framebuffer_queue, &frame_buffer, portMAX_DELAY) != pdPASS) return;
  if (frame_buffer == NULL) return;

  fs::FS &fs = SD_MMC;
  File file = fs.open(path.c_str(), FILE_WRITE);

  if (!file) {
    file.close();
    esp_camera_fb_return(frame_buffer);

    return ESP_FAIL;
  }

  file.write(frame_buffer->buf, frame_buffer->len);
  file.close();

  esp_camera_fb_return(frame_buffer);
  free(frame_buffer);

  PROCESSED_FRAME_COUNT += 1;
}

// Take the photo.
esp_err_t take_photo() {
  esp_err_t err = capture_frame();
  if (err != ESP_OK) return err;

  esp_err_t err = save_frame();
  if (err != ESP_OK) return err;
}
