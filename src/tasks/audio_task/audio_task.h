#ifndef AUDIO_TASK_H_
#define AUDIO_TASK_H_

#include <Arduino.h>
#include <lvgl.h>
#include "Audio.h"
#include "WiFiMulti.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include "driver/gpio.h"
void audio_task(void *pvParam);

#endif