#include <Arduino.h>
#include <Wire.h>
#define SDA_PIN 17 // 指定 SDA 引脚
#define SCL_PIN 18 // 指定 SCL 引脚
TaskHandle_t i2c_task_HANDLE;
void i2c_task(void *pvParam)
{
    Serial.println("\nI2C Scanner");
    Wire.begin(SDA_PIN, SCL_PIN); // 初始化 I2C 总线
    while (1)
    {
        byte error, address;
        int nDevices;

        Serial.println("Scanning...");

        nDevices = 0;
        for (address = 1; address < 127; address++)
        {
            // 使用 Wire 库的 beginTransmission 和 endTransmission 函数扫描 I2C 地址
            Wire.beginTransmission(address);
            //Serial.printf("scan address: %d\n", address);
            error = Wire.endTransmission();

            if (error == 0)
            {
                Serial.print("I2C device found at address 0x");
                if (address < 16)
                    Serial.print("0");
                Serial.print(address, HEX);
                Serial.println("  !");

                nDevices++;
            }
            else if (error == 4)
            {
                Serial.print("Unknown error at address 0x");
                if (address < 16)
                    Serial.print("0");
                Serial.println(address, HEX);
            }
        }
        if (nDevices == 0)
            Serial.println("No I2C devices found\n");
        else
            Serial.println("done\n");

        vTaskDelay(5000); // 5 秒后再次扫描
    }
}