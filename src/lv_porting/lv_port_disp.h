/**
 * @file lv_port_disp_templ.h
 *
 */

/*Copy this file as "lv_port_disp.h" and set this value to "1" to enable content*/
#if 1

#ifndef LV_PORT_DISP_TEMPL_H
#define LV_PORT_DISP_TEMPL_H

#define USE_LGFX 1
#define USE_eTFT 0
#define USE_PSRAM 0
/*********************
 *      INCLUDES
 *********************/
#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
// #include "lvgl/lvgl.h"
#include "lvgl.h"
#endif

#include "Arduino.h"


#if USE_eTFT
#include "TFT_eSPI.h"
#endif


#if USE_LGFX
#include "LGFX_tft.hpp"
#endif


#define MY_DISP_HOR_RES 240//屏幕宽度
#define MY_DISP_VER_RES 320//屏幕高度
// #define MY_DISP_HOR_RES 280
// #define MY_DISP_VER_RES 240
#define LV_VER_RES_MAX  320

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/* Initialize low level display driver */
void lv_port_disp_init(void);

/* Enable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_enable_update(void);

/* Disable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_disable_update(void);

/**********************
 *      MACROS
 **********************/


#endif /*LV_PORT_DISP_TEMPL_H*/

#endif /*Disable/Enable content*/
