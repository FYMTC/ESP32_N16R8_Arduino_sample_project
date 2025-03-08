#include "tasks/LGFX_task/LGFX_task.h"
#include <time.h>
#if 1
#if USE_LGFX==1
extern LGFX_tft tft;
#endif

#if USE_eTFT==1
extern TFT_eSPI tft;
#endif

extern bool page0_flag;
extern lv_timer_t *timer_page0;
extern float battery_voltage;
extern int time_task_size;
extern TaskHandle_t time_task_handle;
extern String songname;
lv_obj_t *label_time;
lv_obj_t *label_second;
lv_obj_t *label_running;
lv_obj_t *label_BATTERY;
lv_obj_t * label_song;
lv_obj_t * label_LRC;
lv_obj_t *old_pages;
lv_obj_t *page0;
// static lv_style_t style;
void page0_time(void *);
void page0_event_cb(lv_event_t *);
const char *timeAPI = "http://worldtimeapi.org/api/timezone/Asia/Shanghai";
void getTimeFromAPI(void);
const char *formatMillis(unsigned long);


// 定义全局变量
struct tm timeinfo;
bool timeNeedsUpdate = false;
bool timeLogUpdate= false;

// 定时器设置
const int timerInterval = 1 * 60 * 1000; // 1 小时，3600000单位为毫秒
unsigned long previousMillis = 0;

void create_page0(lv_event_t *event)
{
    old_pages = lv_scr_act(); // 获得当前活动屏幕
    page0 = lv_obj_create(NULL);

    lv_obj_add_event_cb(page0, page0_event_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_set_style_bg_color(page0, lv_color_black(), 0);
    label_time = lv_label_create(page0);
    lv_label_set_recolor(label_time, true); /*Enable re-coloring by commands in the text*/
    lv_label_set_text(label_time, "#ffffff 00:00#");
    lv_obj_set_style_text_font(label_time, &my_font, 0);
    // lv_obj_set_style_text_font(label_time, &NotoSansSC_Medium_3500, 0);
    lv_obj_center(label_time);
    lv_obj_set_style_text_color(label_time, lv_color_white(), 0);

    lv_scr_load_anim(page0, LV_SCR_LOAD_ANIM_OVER_BOTTOM, 300, LV_SCR_LOAD_ANIM_NONE, false);

    label_second = lv_label_create(page0);
    lv_label_set_recolor(label_second, true); /*Enable re-coloring by commands in the text*/
    lv_label_set_text(label_second, "#ffffff 00-00-00 00:00:00#");
    lv_obj_set_style_text_color(label_second, lv_color_white(), 0);
    lv_obj_align_to(label_second, label_time, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
    
    label_BATTERY = lv_label_create(page0);
    char buf[32];
    sprintf(buf,"%.3f", battery_voltage);
    lv_label_set_text_fmt(label_BATTERY,buf);
    lv_obj_align_to(label_BATTERY, label_second, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    label_running = lv_label_create(page0);
    lv_label_set_text(label_running, formatMillis(millis()));
    lv_obj_align_to(label_running, label_BATTERY, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

    label_song = lv_label_create(page0);
    lv_label_set_text(label_song, " ");
    lv_obj_set_size(label_song, lv_pct(100), lv_pct(6));
    //lv_obj_align_to(label_song, label_running, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    lv_obj_align(label_song, LV_ALIGN_TOP_LEFT, 0, 0);

    label_LRC = lv_label_create(page0);
    lv_obj_set_size(label_LRC, lv_pct(100), lv_pct(10));
    lv_label_set_text(label_LRC, " ");
    lv_obj_align_to(label_LRC, label_song, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);


    // xTaskCreatePinnedToCore(page0_time, "time task", 8*1024, NULL, 1, &time_task, 1);
    xTaskCreate(page0_time, "time task", time_task_size, NULL, 1, &time_task_handle);

    // getTimeFromAPI();
}

void page0_time(void *pvParameters)
{
    for (;;)
    {
        unsigned long currentMillis = millis();

        if (currentMillis - previousMillis >= timerInterval)
        {
            previousMillis = currentMillis;
            timeNeedsUpdate = true;
            timeLogUpdate = true;
        }

        if (timeNeedsUpdate)
        {
            getTimeFromAPI();
            
            if (!page0_flag)
            {
                 #if USE_LGFX==1
                tft.setBrightness(16);
                #endif
            }
        }

        // 获取本地时间
        if (!getLocalTime(&timeinfo))
        {
            Serial.println("Failed to obtain time");
        }

        // 格式化并显示时间
        char strftime_buf[64];
        //strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        //Serial.println(strftime_buf);

        strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
        lv_label_set_text(label_second, strftime_buf);
        if (timeLogUpdate)
        {   
            
            Serial.println(("Current NET time:"+String(strftime_buf)).c_str());
           
            Serial.println(("Current ESP32 local time:"+String(formatMillis(millis()))).c_str());
            
            Serial.println(("Current ESP32 battery_voltage:"+String(battery_voltage)).c_str());
            timeLogUpdate = false;
        }

        strftime(strftime_buf, sizeof(strftime_buf), "%H:%M", &timeinfo);
        lv_label_set_text(label_time, strftime_buf);

        lv_label_set_text(label_running, formatMillis(millis()));
        lv_label_set_text(label_song, songname.c_str());

        char buf[32];
        sprintf(buf,"%.3f", battery_voltage);
        lv_label_set_text_fmt(label_BATTERY,buf);

        vTaskDelay(pdMS_TO_TICKS(1000)); // 每秒刷新一次时间
    }
}

void getTimeFromAPI()
{
    HTTPClient http_time;

    // Make HTTP GET request to time API
    http_time.begin(timeAPI);
    int httpCode = http_time.GET();

    // Check for successful connection
    if (httpCode > 0)
    {
        String payload = http_time.getString();
        // Serial.println("Response payload: " + payload);

        // Parse JSON response
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);
        if (error)
        {
            Serial.println("Failed to parse JSON");
            return;
        }

        // Extract datetime from JSON
        const char *datetime = doc["datetime"];
        Serial.println("time: " + String(datetime));

        unsigned long epochTime = doc["unixtime"];
        timeNeedsUpdate = false;
        // 更新本地时间
        struct timeval tv;
        tv.tv_sec = epochTime + 8 * 60 * 60;
        tv.tv_usec = 0;
        settimeofday(&tv, NULL);
        // 打印当前时间
        Serial.println("Current time: " + String(epochTime));
        
    }
    else
    {
        Serial.println("Error on HTTP request");
    }
    http_time.end();
}

const char *formatMillis(unsigned long millis)
{
    // 计算小时、分钟和秒
    unsigned long seconds = millis / 1000;
    unsigned long hours = seconds / 3600;
    seconds %= 3600;
    unsigned long minutes = seconds / 60;
    seconds %= 60;

    // 创建格式化字符串
    static char buffer[9]; // HH:MM:SS 加上空字符结尾
    snprintf(buffer, sizeof(buffer), "%02lu:%02lu:%02lu", hours, minutes, seconds);

    return buffer; // 返回格式化后的字符串
}

void page0_event_cb(lv_event_t *event)
{ // 页面切换回调函数，目标页面由用户数据决定,旧页面被卸载
    Serial.println("[page_manage] page_event_cb page0->manu");
    page0_flag = true;
    lv_timer_reset(timer_page0);
    // vTaskDelete(time_task);
    // if (time_task_handel != NULL) {
    //     vTaskDelete(time_task_handel);
    //     time_task_handel = NULL; // 清除句柄
    // }
    // lv_scr_load_anim(old_pages, LV_SCR_LOAD_ANIM_OVER_TOP, 100, 300, true);
    lv_scr_load_anim(old_pages, LV_SCR_LOAD_ANIM_OVER_TOP, 100, 300, false);
    #if USE_LGFX==1
    tft.setBrightness(64);
    #endif

}


#endif