#include "tasks/LGFX_task/LGFX_task.h"
#if USE_LGFX == 1
extern LGFX_tft tft;
#endif

#if USE_eTFT == 1
extern TFT_eSPI tft;
#endif
#define LGFX_AUTODETECT
lv_timer_t *timer_page0;
float battery_voltage = 0;
bool page0_flag = 1;
extern TaskHandle_t time_task_handle;
extern lv_obj_t *page0;
lv_obj_t *page_LGFX;
void page_LGFX_event_cb(lv_event_t *event);

void timer_page0_cb(lv_timer_t *timer)
{
  if (page0_flag && time_task_handle == NULL)
  {
    create_page0(NULL);
    page0_flag = false;
    Serial.println("[page manage]page_menu->page0");
  }
  else if (page0_flag && time_task_handle != NULL)
  {
    lv_scr_load_anim(page0, LV_SCR_LOAD_ANIM_OVER_BOTTOM, 300, LV_SCR_LOAD_ANIM_NONE, false);
    page0_flag = false;
    Serial.println("[page manage]page_menu->page0");
  }
}

void LGFX_task(void *pvParam)
{

  Serial.println("[LGFX task] init lvgl...");
  lv_init();
  lv_port_disp_init();
  lv_port_indev_init();
  /**/
  tft.fillScreen(TFT_RED);
  vTaskDelay(1000);

  tft.fillScreen(TFT_GREEN);
  vTaskDelay(1000);

  tft.fillScreen(TFT_BLUE);
  vTaskDelay(1000);

  //page_LGFX = lv_obj_create(NULL);
  // timer_page0 = lv_timer_create(timer_page0_cb, 30000, NULL); // 无操作超时30s自动跳转page0(时钟页面)定时器
  // lv_timer_set_repeat_count(timer_page0, -1);
  // lv_obj_add_event_cb(page_LGFX, page_LGFX_event_cb, LV_EVENT_PRESSED, NULL);

  // lv_obj_t *song_cover;
  // song_cover = lv_img_create(page_LGFX);
  // lv_img_set_src(song_cover, "/:song_cover.jpg");
  // lv_img_set_size_mode(song_cover, LV_IMG_SIZE_MODE_REAL);
  // lv_obj_set_size(song_cover, 200,200);
  // lv_img_set_zoom(song_cover, 64);
  // lv_img_set_antialias(song_cover, true);
  // lv_obj_align(song_cover, LV_ALIGN_CENTER, 0, 0);
  // lv_obj_set_style_radius(song_cover,64,0);
  //lv_disp_load_scr(page_LGFX);
  
  Serial.println("[LGFX task] lvgl lv_demo begin");
  lv_demo_benchmark();
  // lv_demo_stress();

  // while (1)
  //  {
  // lv_timer_handler();
  //    vTaskDelay(1);
  //}
}
void page_LGFX_event_cb(lv_event_t *event)
{
  page0_flag = true;
  lv_timer_reset(timer_page0);
#if USE_LGFX == 1
  tft.setBrightness(64);
#endif
}
