#ifndef TFT_TASK_H
#define TFT_TASK_H

#include <lvgl.h>
#include "lv_porting/lv_port_disp.h"

#include "demos/lv_demos.h"
#include "TCPReceiver.h"
#include "DMADrawer.h"

void tft_task(void *pvParam);

#endif