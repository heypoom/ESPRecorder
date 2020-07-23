#include <Arduino.h>

#include "../lib/config.h"
#include "../lib/leds.h"

// Notify the human operator then restart the device.
void panic() {
  notify_critical_error();
  esp_restart();
}

// Notify the human operator that an unrecoverable error exists.
// This will last 5 seconds.
void notify_critical_error() {
  // Some LED and flash to grab human attention that something is wrong.

  for (int i = 0; i < 5; i++) {
    led_on();
    flash_on();

    delay(500);

    led_off();
    flash_off();

    delay(500);
  }
}

// Notify the human operator that something is not right, but not critical.
// Will last 3 seconds.
void notify_warning() {
  for (int i = 0; i < 3; i++) {
    led_on();

    delay(500);

    led_off();

    delay(500);
  }
}