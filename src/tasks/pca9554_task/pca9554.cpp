#include <Adafruit_XCA9554.h>
#include "pca9554.h"

// 定义 TP_INT 和 TP_RST 引脚在 IO 扩展芯片上的编号
#define TP_INT_PIN 6 // P6
#define TP_RST_PIN 7 // P7

#define CDC_MODE 0
#define CDC_IO1 1

// Create an instance of the Adafruit_XCA9554 class
Adafruit_XCA9554 io_expander;
// 复位触摸屏
void reset_touch_screen()
{
    // 配置 TP_RST 引脚为输出模式
    if (!io_expander.pinMode(TP_RST_PIN, OUTPUT))
    {
        Serial.println("TP_RST 引脚配置失败！");
        while (1)
            ;
    }

    // 配置 TP_INT 引脚为输入模式
    if (!io_expander.pinMode(TP_INT_PIN, INPUT))
    {
        Serial.println("TP_INT 引脚配置失败！");
        while (1)
            ;
    }
    // 拉低 TP_RST 引脚
    if (!io_expander.digitalWrite(TP_RST_PIN, LOW))
    {
        Serial.println("TP_RST 引脚拉低失败！");
        return;
    }
    delay(10); // 保持低电平 10ms

    // 拉高 TP_RST 引脚
    if (!io_expander.digitalWrite(TP_RST_PIN, HIGH))
    {
        Serial.println("TP_RST 引脚拉高失败！");
        return;
    }
    delay(100); // 等待触摸屏初始化完成

    Serial.println("触摸屏复位完成！");
}
void set_nau88c22_mode(){

    if (!io_expander.pinMode(CDC_MODE, OUTPUT))
    {
        Serial.println("CDC_MODE 引脚配置失败！");
        while (1)
            ;
    }
    if (!io_expander.digitalWrite(CDC_MODE, LOW))
    {
        Serial.println("CDC_MODE 引脚拉低失败！");
        return;
    }
    if (!io_expander.pinMode(CDC_IO1, OUTPUT))
    {
        Serial.println("CDC_IO1 引脚配置失败！");
        while (1)
            ;
    }
    if (!io_expander.digitalWrite(CDC_IO1, LOW))
    {
        Serial.println("CDC_IO1 引脚拉低失败！");
        return;
    }

}
void pca9554_task(void *pvParameters)
{
    Serial.println("PCA/TCA9554 Simple Test Sketch");
    // Begin communication with the expander
    if (!io_expander.begin(0x38, &Wire))
    { // Replace with actual I2C address if different
        Serial.println("Failed to find XCA9554 chip");
        while (1)
            ;
    }
    Serial.println("IO 扩展芯片初始化成功！");

    // 复位触摸屏
    reset_touch_screen();

    set_nau88c22_mode();
    while (1)
    {
        vTaskDelay(10); // debounce delay
        vTaskDelete(NULL); // 删除任务
    }
}