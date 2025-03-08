#include "RGB_tasks.h"

#define LED_PIN 48
#define LED_COUNT 1

NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0800KbpsMethod> strip(LED_COUNT, LED_PIN);

volatile bool flag = false;

void handleInterrupt() {
    flag = true;
}

//xTaskCreate(RGBtask,"RGB LED",1*1024,NULL,1,NULL);

void RGBtask(void *pvParam) {
    Serial.println(F("[FreeRTOS]RGBtask BEGIN"));
    pinMode(LED_BUILTIN, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(0), handleInterrupt, FALLING);

    strip.Begin();
    strip.Show();

    while(1){
    //if (flag) {
        //flag = false;
        RgbColor red(8, 0, 0);
        RgbColor green(0, 8, 0);
        RgbColor blue(0, 0, 8);
        RgbColor redgreen(8, 8, 0);
        RgbColor redblue(8, 0, 8);
        RgbColor bluegreen(0, 8, 8);
        RgbColor redbluegreen(8, 8, 8);
        
        // Red to green transition
        for (float i = 0; i <= 1; i += 0.01) {
            RgbColor color = RgbColor::LinearBlend(red, redgreen, i);
            strip.ClearTo(color);
            strip.Show();
            vTaskDelay(10);
        }
        
        // Green to blue transition
        for (float i = 0; i <= 1; i += 0.01) {
            RgbColor color = RgbColor::LinearBlend(redgreen, redblue, i);
            strip.ClearTo(color);
            strip.Show();
            vTaskDelay(10);
        }
        
        // Blue to red transition
        for (float i = 0; i <= 1; i += 0.01) {
            RgbColor color = RgbColor::LinearBlend(redblue, blue, i);
            strip.ClearTo(color);
            strip.Show();
            vTaskDelay(10);
        }

        for (float i = 0; i <= 1; i += 0.01) {
            RgbColor color = RgbColor::LinearBlend(blue, bluegreen, i);
            strip.ClearTo(color);
            strip.Show();
            vTaskDelay(10);
        }

        for (float i = 0; i <= 1; i += 0.01) {
            RgbColor color = RgbColor::LinearBlend(bluegreen, green, i);
            strip.ClearTo(color);
            strip.Show();
            vTaskDelay(10);
        }

        for (float i = 0; i <= 1; i += 0.01) {
            RgbColor color = RgbColor::LinearBlend(green, redbluegreen, i);
            strip.ClearTo(color);
            strip.Show();
            vTaskDelay(10);
        }
        
         for (float i = 0; i <= 1; i += 0.01) {
            RgbColor color = RgbColor::LinearBlend(redbluegreen, red, i);
            strip.ClearTo(color);
            strip.Show();
            vTaskDelay(10);
        }
        
    // }
    }
}
