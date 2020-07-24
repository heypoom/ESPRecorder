#include <Arduino.h>

#define FLASH_LED_PIN 4
#define RED_LED_PIN 33

#define PIN_START_RECORDING_SIGNAL GPIO_NUM_3
#define PIN_STOP_RECORDING_SIGNAL GPIO_NUM_1

#define EEPROM_SIZE 1

#define BROWNOUT_DET_LVL CONFIG_BROWNOUT_DET_LVL
#define CONFIG_BROWNOUT_DET_LVL_SEL_5 1

#define DELAY_BETWEEN_FRAMES_MSEC 100
#define ENABLE_SERIAL_LOG false

#define FRAME_BUFFER_QUEUE_SIZE 30
