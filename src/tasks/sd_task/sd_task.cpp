#include "Arduino.h"
#include "SD_MMC.h"
bool SD_INT_STATUE;
// 自定义引脚号
#define SDMMC_CLK 7
#define SDMMC_CMD 6
#define SDMMC_D0  8
#define SDMMC_D1  9
#define SDMMC_D2  4
#define SDMMC_D3  5
SemaphoreHandle_t sdCardMutex; // Mutex to protect SD card access
// SD卡检测引脚
#define SD_DET_PIN 10  // GPIO10 用于检测SD卡是否存在
// 检测SD卡是否存在
bool isSDCardPresent() {
    // 如果检测引脚为低电平，表示SD卡存在
    return digitalRead(SD_DET_PIN) == LOW;
  }
// 列出目录中的文件
void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
    Serial.printf("列出目录: %s\n", dirname);
  
    File root = fs.open(dirname);
    if (!root) {
      Serial.println("无法打开目录");
      return;
    }
    if (!root.isDirectory()) {
      Serial.println("不是目录");
      return;
    }
  
    File file = root.openNextFile();
    while (file) {
      if (file.isDirectory()) {
        Serial.print("  DIR : ");
        Serial.println(file.name());
        if (levels) {
          listDir(fs, file.name(), levels - 1);
        }
      } else {
        Serial.print("  FILE: ");
        Serial.print(file.name());
        Serial.print("  SIZE: ");
        Serial.println(file.size());
      }
      file = root.openNextFile();
    }
  }
void sd_setup() {
  // 初始化SD卡检测引脚
  pinMode(SD_DET_PIN, INPUT_PULLUP);  // 使用内部上拉电阻

  // 检测SD卡是否存在
  if (!isSDCardPresent()) {
    Serial.println("未检测到SD卡，请插入SD卡！");
    SD_INT_STATUE=false;
    return;
  }
  SD_INT_STATUE=true;

  // 设置自定义引脚
  if (!SD_MMC.setPins(SDMMC_CLK, SDMMC_CMD, SDMMC_D0, SDMMC_D1, SDMMC_D2, SDMMC_D3)) {
    Serial.println("引脚设置失败！");
    return;
  }

  // 初始化SDMMC
  if (!SD_MMC.begin("/sdcard", true, true)) {
    Serial.println("SDMMC初始化失败！");
    return;
  }

  Serial.println("SDMMC初始化成功！");

  // 列出根目录下的文件
  //listDir(SD_MMC, "/", 0);
}