#include <Arduino.h>

#include "../lib/config.h"

void led_on() {
  digitalWrite(RED_LED_PIN, LOW);
}

void led_off() {
  digitalWrite(RED_LED_PIN, HIGH);
}

void flash_on() {
  digitalWrite(FLASH_LED_PIN, HIGH);
}

void flash_off() {
  digitalWrite(FLASH_LED_PIN, LOW);
}