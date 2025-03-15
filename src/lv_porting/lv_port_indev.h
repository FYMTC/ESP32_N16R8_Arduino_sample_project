
/**
 * @file lv_port_indev_templ.h
 *
 */

/*Copy this file as "lv_port_indev.h" and set this value to "1" to enable content*/
#if 1

#ifndef LV_PORT_INDEV_TEMPL_H
#define LV_PORT_INDEV_TEMPL_H

// #ifdef __cplusplus
// extern "C" {
// #endif

/*********************
 *      INCLUDES
 *********************/
// #include "lvgl/lvgl.h"
#include "lvgl.h"
#include "Arduino.h"
#include <Ticker.h>
#include <driver/pcnt.h>
/*********************
 *      DEFINES
 *********************/
// 编码器引脚定义
#define ENCODER_A_PIN 16  // A 相连接到 GPIO16
#define ENCODER_B_PIN 2   // B 相连接到 GPIO2
#define ENCODER_BT_PIN 3  // 按钮连接到 GPIO3

// PCNT 单元和通道
#define PCNT_UNIT PCNT_UNIT_0
#define PCNT_CHANNEL PCNT_CHANNEL_0
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void lv_port_indev_init(void);

/**********************
 *      MACROS
 **********************/

// #ifdef __cplusplus
// } /*extern "C"*/
// #endif

#endif /*LV_PORT_INDEV_TEMPL_H*/

#endif /*Disable/Enable content*/
