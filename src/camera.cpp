#include "../lib/camera.h"

#include <Arduino.h>
#include <EEPROM.h>
#include <SD_MMC.h>

#include "../lib/config.h"
#include "../lib/notify_error.h"
#include "esp_camera.h"

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
    // TODO: PSRAM is not found! Flash a warning.
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
    Serial.printf("Camera init failed with error 0x%x", err);

    return ESP_FAIL;
  }

  if (!SD_MMC.begin()) {
    Serial.println("SD Card Mount Failed");

    return ESP_FAIL;
  }

  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD Card attached");

    return ESP_FAIL;
  }

  return ESP_OK;
}

esp_err_t take_photo(String file_name) {
  camera_fb_t *frame_buffer = NULL;

  // Take a photo!
  frame_buffer = esp_camera_fb_get();
  if (!frame_buffer) {
    Serial.println("Camera capture failed");

    return ESP_FAIL;
  }

  // Path where new picture will be saved in SD Card
  String path = "/" + file_name + ".jpg";

  fs::FS &fs = SD_MMC;
  Serial.printf("Picture file name: %s\n", path.c_str());

  File file = fs.open(path.c_str(), FILE_WRITE);

  if (!file) {
    Serial.println("Failed to open file in writing mode");

    file.close();
    esp_camera_fb_return(frame_buffer);

    return ESP_FAIL;
  }

  file.write(frame_buffer->buf, frame_buffer->len);
  Serial.printf("Saved file to path: %s\n", path.c_str());

  file.close();
  esp_camera_fb_return(frame_buffer);

  return ESP_OK;
}
