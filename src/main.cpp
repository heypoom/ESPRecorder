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
#include "soc/timer_group_reg.h"
#include "soc/timer_group_struct.h"

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

// Update the TAKE_ID.
void start_new_take() {
  TAKE_ID = EEPROM.read(0) + 1;
  EEPROM.write(0, TAKE_ID);
  EEPROM.commit();
}

// Continuously take the photo if IS_RECORDING flag is set.
void continuouslyTakePhotoTask(void *parameter) {
  while (true) {
    if (IS_RECORDING) {
      String photo_name = "take_" + String(TAKE_ID) + "_frame_" + String(FRAME_COUNT);

      // Take the photo!
      esp_err_t err = take_photo(photo_name);
      if (err != ESP_OK) {
        // Nothing we can do.
      }

      FRAME_COUNT++;
    } else {
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }
  }
}

void create_photo_task() {
  int heap = 4096;
  int cpu_core = 0;
  int priority = 2;

  xTaskCreatePinnedToCore(continuouslyTakePhotoTask, "CameraTask", heap, NULL, priority, &CameraTask, cpu_core);
}

void setup() {
  Serial.begin(115200);

  // Initialize the take ID from EEPROM.
  EEPROM.begin(EEPROM_SIZE);

  // Use red LED indicator and start at OFF
  pinMode(RED_LED_PIN, OUTPUT);
  led_off();

  // Use flash LED and start at OFF
  pinMode(FLASH_LED_PIN, OUTPUT);
  flash_off();

  // Disable brownout detector.
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  // Configure watchdog.
  TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;
  TIMERG0.wdt_feed = 1;
  TIMERG0.wdt_wprotect = 0;

  TIMERG1.wdt_wprotect = TIMG_WDT_WKEY_VALUE;
  TIMERG1.wdt_feed = 1;
  TIMERG1.wdt_wprotect = 0;

  // Listen to GPIO pins for start and stop signal.
  pinMode(PIN_START_RECORDING_SIGNAL, INPUT);
  pinMode(PIN_STOP_RECORDING_SIGNAL, INPUT);

  // Setup the built-in camera.
  esp_err_t err = setup_camera();
  if (err != ESP_OK) notify_panic();

  // Run the photo task on separate core.
  create_photo_task();

  // Set the LED to ON if everything looks good.
  led_on();
}

void start_recording() {
  if (IS_RECORDING) return;

  start_new_take();
  flash_on();

  IS_RECORDING = true;
}

void stop_recording() {
  if (!IS_RECORDING) return;

  flash_off();

  IS_RECORDING = false;
}

void debug_simulate_signal_with_serial() {
  if (Serial.available() > 0) {
    int command = Serial.read();

    Serial.print("I received: ");
    Serial.println(command, DEC);

    if (command == 48) {
      Serial.println("STOP RECORDING.");
      stop_recording();
    } else if (command == 49) {
      Serial.println("START RECORDING.");
      start_recording();
    }
  }
}

void read_gpio_signals() {
  int isStopRecordingSignal = digitalRead(PIN_STOP_RECORDING_SIGNAL);

  if (isStopRecordingSignal == HIGH) {
    stop_recording();
    return;
  }

  int isStartRecordingSignal = digitalRead(PIN_STOP_RECORDING_SIGNAL);

  if (isStartRecordingSignal == HIGH) {
    start_recording();
  }
}

void loop() {
  // debug_simulate_signal_with_serial();
  read_gpio_signals();
  delay(10);
}
