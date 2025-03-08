#ifndef LGFX_TASK_H_
#define LGFX_TASK_H_
#include <lvgl.h>
#include "lv_porting/lv_port_disp.h"
#include "lv_porting/lv_port_indev.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include "demos/lv_demos.h"
#include <wifi.h>
#include <HTTPClient.h>
LV_FONT_DECLARE(my_font);
void create_page0(lv_event_t*);
void LGFX_task(void *pvParam);




#endif /* LGFX_TASK_H_ */