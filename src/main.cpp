#include <Arduino.h>
#include <EEPROM.h>

// Camera

#include "esp_camera.h"

// Config
#include "../lib/camera.h"
#include "../lib/config.h"
#include "../lib/leds.h"
#include "../lib/main.h"
#include "../lib/notify_error.h"

// SD Card
#include <SD_MMC.h>

#include "FS.h"
#include "driver/sdmmc_defs.h"
#include "driver/sdmmc_host.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

// SOC
#include "driver/rtc_io.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/soc.h"

// Task Management
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "soc/cpu.h"

// Global Variables!

// Task handle.
TaskHandle_t CameraTask;

// Are we recording?
int IS_RECORDING = false;

// ID to prevent collision in file name.
int TAKE_ID = 0;

// Which frame are we currently on?
int FRAME_COUNT = 0;

// Start a new take on device restart.
void start_new_take() {
  TAKE_ID = EEPROM.read(0) + 1;
  EEPROM.write(0, TAKE_ID);
  EEPROM.commit();
}

void takePhotoTask(void *parameter) {
  for (;;) {
    if (IS_RECORDING) {
      String photo_name = "take_" + String(TAKE_ID) + "_frame_" + String(FRAME_COUNT);

      esp_err_t err = take_photo(photo_name);
      delay(100);

      FRAME_COUNT++;
    }
  }
}

void create_photo_task() {
  int heap = 4096;
  int cpu_core = 1;
  int priority = 2;

  xTaskCreatePinnedToCore(takePhotoTask, "CameraTask", heap, NULL, priority, &CameraTask, cpu_core);
}

void setup() {
  Serial.begin(115200);

  // Initialize the take ID from EEPROM.
  EEPROM.begin(EEPROM_SIZE);
  start_new_take();

  // Use red LED indicator and start at ON
  pinMode(RED_LED_PIN, OUTPUT);
  led_on();

  // Use flash LED and start at OFF
  pinMode(FLASH_LED_PIN, OUTPUT);
  flash_off();

  // Disable brownout detector.
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  esp_err_t err = setup_camera();
  if (err != ESP_OK) panic();

  // rtc_gpio_hold_en(GPIO_NUM_4);

  create_photo_task();
}

void loop() {
  
}
