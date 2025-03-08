#include "tasks/audio_task/audio_task.h"
#include "driver/gpio.h"
// I2S引脚配置
#define I2S_MCLK_PIN 0  // I2S0_MCLK 0，1，3
#define I2S_BCLK_PIN 45 // I2S0_SCLK
#define I2S_LRC_PIN 47  // I2S0_LRCK
#define I2S_DSIN_PIN 40 // I2S0_DSDIN

#define MAX_LINES 100       // Adjust according to the expected maximum number of lines
#define MAX_LINE_LENGTH 128 // Maximum length of each line
extern SemaphoreHandle_t sdCardMutex;
extern lv_obj_t *label_LRC;
extern bool page0_flag;
bool readFlag=false;
extern lv_obj_t *page_LGFX;
String lrcFileName;
// 定义音频解码器和I2S输出
Audio audio;
WiFiMulti wifiMulti;
int currentTime;
String songname;
String Audiossid = "404";
String Audiopassword = "abcd0404";

int lyricsCount = 0;
char *text_buffer = nullptr;
int *timestamps = nullptr;
char **lyrics = nullptr;
// Buffer size for reading the image
#define IMAGE_BUFFER_SIZE 1024

void audio_id3image(File &file, const size_t pos, const size_t size)
{ // cover image
    Serial.printf("id3image found at pos: %u, length: %u\n", pos, size);
    uint8_t buf[1024];
    file.seek(pos + 1); // skip 1 byte encoding
    char mimeType[255]; // mime-type (null terminated)
    for (uint8_t i = 0u; i < 255; i++)
    {
        mimeType[i] = file.read();
        if (uint8_t(mimeType[i]) == 0)
            break;
    }
    Serial.printf("MimeType: %s\n", mimeType);
    uint8_t imageType = file.read(); // image type (1 Byte)
    Serial.printf("ImageType: %d\n", imageType);
    for (uint8_t i = 0u; i < 255; i++)
    { // description (null terminated)
        buf[i] = file.read();
        if (uint8_t(buf[i]) == 0)
            break;
    }
    // raw image data
    if (xSemaphoreTake(sdCardMutex, portMAX_DELAY) == pdTRUE)
    {
        File coverFile = SD.open("/cover.jpg", FILE_WRITE);
        size_t len = size;
        while (len)
        {
            uint16_t bytesRead = file.read(buf, sizeof(buf));
            if (len >= bytesRead)
                len -= bytesRead;
            else
            {
                bytesRead = len;
                len = 0;
            }
            coverFile.write(buf, bytesRead);
        }
        Serial.println("Cover file written");
        coverFile.close();
        xSemaphoreGive(sdCardMutex); // Release the mutex
    }
}

void display_image_with_lvgl(const char *path)
{
    lv_obj_t *img = lv_img_create(lv_scr_act()); // Create an image object
    lv_img_set_src(img, path);                   // Set the source of the image
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);    // Align the image to the center
}
bool findImageInMP3(File &mp3File, size_t &imagePosition, size_t &imageSize)
{
    // Ensure the file is at the beginning
    mp3File.seek(0);

    // Read the first 10 bytes to check for the ID3v2 tag
    char header[10];
    mp3File.read((uint8_t *)header, 10);

    // Check for "ID3" identifier
    if (strncmp(header, "ID3", 3) != 0)
    {
        Serial.println("No ID3v2 tag found.");
        return false;
    }

    // Get the size of the ID3 tag
    size_t tagSize = ((header[6] & 0x7F) << 21) |
                     ((header[7] & 0x7F) << 14) |
                     ((header[8] & 0x7F) << 7) |
                     (header[9] & 0x7F);

    size_t pos = 10; // Start after the ID3v2 header

    while (pos < tagSize + 10)
    {
        // Read the next frame header
        char frameHeader[10];
        mp3File.seek(pos);
        mp3File.read((uint8_t *)frameHeader, 10);

        // Get the frame ID (4 bytes)
        char frameID[5];
        strncpy(frameID, frameHeader, 4);
        frameID[4] = '\0';

        // Get the size of the frame (4 bytes, syncsafe integers)
        size_t frameSize = ((frameHeader[4] & 0xFF) << 24) |
                           ((frameHeader[5] & 0xFF) << 16) |
                           ((frameHeader[6] & 0xFF) << 8) |
                           (frameHeader[7] & 0xFF);

        // Check if it's the APIC frame
        if (strcmp(frameID, "APIC") == 0)
        {
            Serial.printf("APIC frame found at position: %u, size: %u\n", pos, frameSize);

            // Skip the text encoding (1 byte) and MIME type (until the null byte)
            mp3File.seek(pos + 11); // Skip past the 10-byte frame header and 1-byte encoding
            while (mp3File.read() != 0)
            {
            }

            // Skip image type (1 byte) and description (null-terminated string)
            mp3File.read(); // Skip image type
            while (mp3File.read() != 0)
            {
            }

            // The image data starts here
            imagePosition = mp3File.position();
            imageSize = frameSize - (imagePosition - pos - 10);

            return true;
        }

        // Move to the next frame
        pos += 10 + frameSize;
    }

    Serial.println("No APIC frame found.");
    return false;
}

void displayImageFromMP3(File &mp3File)
{
    // Ensure the file is at the beginning
    mp3File.seek(0);

    // Read the first 10 bytes to check for the ID3v2 tag
    char header[10];
    mp3File.read((uint8_t *)header, 10);

    // Check for "ID3" identifier
    if (strncmp(header, "ID3", 3) != 0)
    {
        Serial.println("No ID3v2 tag found.");
        return;
    }

    // Get the size of the ID3 tag
    size_t tagSize = ((header[6] & 0x7F) << 21) |
                     ((header[7] & 0x7F) << 14) |
                     ((header[8] & 0x7F) << 7) |
                     (header[9] & 0x7F);

    size_t pos = 10; // Start after the ID3v2 header
    size_t imagePosition = 0;
    size_t imageSize = 0;

    while (pos < tagSize + 10)
    {
        // Read the next frame header
        char frameHeader[10];
        mp3File.seek(pos);
        mp3File.read((uint8_t *)frameHeader, 10);

        // Get the frame ID (4 bytes)
        char frameID[5];
        strncpy(frameID, frameHeader, 4);
        frameID[4] = '\0';

        // Get the size of the frame (4 bytes, syncsafe integers)
        size_t frameSize = ((frameHeader[4] & 0xFF) << 24) |
                           ((frameHeader[5] & 0xFF) << 16) |
                           ((frameHeader[6] & 0xFF) << 8) |
                           (frameHeader[7] & 0xFF);

        // Check if it's the APIC frame
        if (strcmp(frameID, "APIC") == 0)
        {
            Serial.printf("APIC frame found at position: %u, size: %u\n", pos, frameSize);

            // Skip the text encoding (1 byte) and MIME type (until the null byte)
            mp3File.seek(pos + 11); // Skip past the 10-byte frame header and 1-byte encoding
            while (mp3File.read() != 0)
            {
            }

            // Skip image type (1 byte) and description (null-terminated string)
            mp3File.read(); // Skip image type
            while (mp3File.read() != 0)
            {
            }

            // The image data starts here
            imagePosition = mp3File.position();
            imageSize = frameSize - (imagePosition - pos - 10);

            break;
        }

        // Move to the next frame
        pos += 10 + frameSize;
    }

    if (imagePosition == 0 || imageSize == 0)
    {
        Serial.println("No APIC frame (image) found.");
        return;
    }

    // Allocate a buffer for the image
    uint8_t *image_buffer = (uint8_t *)ps_malloc(imageSize);
    if (image_buffer == nullptr)
    {
        Serial.println("Failed to allocate memory for image buffer.");
        return;
    }

    // Read the image data into the buffer
    mp3File.seek(imagePosition);
    size_t bytesRead = 0;
    while (bytesRead < imageSize)
    {
        size_t len = min(imageSize - bytesRead, (size_t)IMAGE_BUFFER_SIZE); // Corrected line
        mp3File.read(image_buffer + bytesRead, len);
        bytesRead += len;
    }

    Serial.printf("Image data read, total bytes: %d\n", bytesRead);

    // Display the image using LVGL
    lv_obj_t *img = lv_img_create(lv_scr_act());
    lv_img_set_src(img, image_buffer);
    // lv_img_set_size_mode(img, LV_IMG_SIZE_MODE_REAL);
    // lv_img_set_zoom(img, 255);
    lv_obj_set_size(img, 100,100);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);

    // Free the buffer after displaying the image
    free(image_buffer);
}

void readAndParseLRCFile(const char *filename)
{
    if (xSemaphoreTake(sdCardMutex, portMAX_DELAY) == pdTRUE)
    {
        // Open the LRC file
        File lyricsFile = SD.open(filename);
        if (!lyricsFile)
        {
            Serial.println("Failed to open file.");
            return;
        }

        // Allocate a buffer for the file content
        size_t fileSize = lyricsFile.size();
        text_buffer = (char *)ps_malloc(fileSize + 1); // +1 for the null terminator
        if (text_buffer == nullptr)
        {
            Serial.println("PSRAM allocation failed for text buffer, falling back to RAM.");
            text_buffer = (char *)malloc(fileSize + 1); // Attempt to allocate in RAM
            if (text_buffer == nullptr)
            {
                Serial.println("Failed to allocate memory for file content.");
                lyricsFile.close();
                xSemaphoreGive(sdCardMutex); // Release the mutex
                return;
            }
        }

        // Read the file content into the buffer
        lyricsFile.read((uint8_t *)text_buffer, fileSize); // Cast
        text_buffer[fileSize] = '\0';                      // Null-terminate the buffer
        lyricsFile.close();
        xSemaphoreGive(sdCardMutex); // Release the mutex
    }
    // Allocate arrays to store timestamps and lyrics
    timestamps = (int *)ps_malloc(MAX_LINES * sizeof(int));
    if (timestamps == nullptr)
    {
        Serial.println("PSRAM allocation failed for timestamps, falling back to RAM.");
        timestamps = (int *)malloc(MAX_LINES * sizeof(int));
        if (timestamps == nullptr)
        {
            Serial.println("Failed to allocate memory for timestamps.");
            free(text_buffer); // Clean up previously allocated memory
            return;
        }
    }

    lyrics = (char **)ps_malloc(MAX_LINES * sizeof(char *));
    if (lyrics == nullptr)
    {
        Serial.println("PSRAM allocation failed for lyrics, falling back to RAM.");
        lyrics = (char **)malloc(MAX_LINES * sizeof(char *));
        if (lyrics == nullptr)
        {
            Serial.println("Failed to allocate memory for lyrics.");
            free(text_buffer); // Clean up previously allocated memory
            free(timestamps);  // Clean up previously allocated memory
            return;
        }
    }

    // Parse the buffer line by line
    char *line = strtok(text_buffer, "\n");
    while (line != nullptr && lyricsCount < MAX_LINES)
    {
        int bracketOpen = -1;
        int bracketClose = -1;

        // Find the positions of the brackets
        for (int i = 0; line[i] != '\0'; i++)
        {
            if (line[i] == '[')
                bracketOpen = i;
            if (line[i] == ']')
            {
                bracketClose = i;
                break;
            }
        }

        if (bracketOpen != -1 && bracketClose != -1 && bracketClose > bracketOpen + 1)
        {
            // Extract the timestamp
            char timeStr[6] = {0}; // Format: mm:ss
            strncpy(timeStr, line + bracketOpen + 1, bracketClose - bracketOpen - 1);

            int minutes = atoi(timeStr);
            int seconds = atoi(timeStr + 3);
            int totalTime = minutes * 60 + seconds;

            // Store the timestamp
            timestamps[lyricsCount] = totalTime;

            // Store the lyric part
            lyrics[lyricsCount] = strdup(line + bracketClose + 1); // Duplicate the lyric string
            if (lyrics[lyricsCount] == nullptr)
            {
                Serial.println("Failed to allocate memory for a lyric line.");
                break;
            }

            lyricsCount++;
        }

        // Get the next line
        line = strtok(nullptr, "\n");
    }

    // Free the text buffer as it's no longer needed
    free(text_buffer);
    readFlag = true;
}

void freeLyricsData()
{
    // Free allocated memory for lyrics
    if (lyrics != nullptr)
    {
        for (int i = 0; i < lyricsCount; i++)
        {
            if (lyrics[i] != nullptr)
            {
                free(lyrics[i]);
            }
        }
        free(lyrics);
        lyrics = nullptr;
    }

    // Free the timestamps array
    if (timestamps != nullptr)
    {
        free(timestamps);
        timestamps = nullptr;
    }
}

void audio_task(void *pvParam)
{

    // 初始化音频库
    WiFi.mode(WIFI_STA);
    wifiMulti.addAP(Audiossid.c_str(), Audiopassword.c_str());
    wifiMulti.run();
    if (WiFi.status() != WL_CONNECTED)
    {
        WiFi.disconnect(true);
        wifiMulti.run();
    }
    audio.i2s_mclk_pin_select(I2S_MCLK_PIN);
    audio.setPinout(I2S_BCLK_PIN, I2S_LRC_PIN, I2S_DSIN_PIN);
    audio.setVolume(2); // 0...21

    Serial.println("****AUDIO START****");
    // audio.connecttoFS(SD, "/Haru.mp3");
    // audio.connecttoSD("/Stay Alive.mp3");
    //     audio.connecttoFS(SD, "test.wav");
    // audio.connecttohost("http://www.wdr.de/wdrlive/media/einslive.m3u");
    //    audio.connecttohost("http://macslons-irish-pub-radio.com/media.asx");
    //    audio.connecttohost("http://mp3.ffh.de/radioffh/hqlivestream.aac"); //  128k aac
    // audio.connecttohost("http://mp3.ffh.de/radioffh/hqlivestream.mp3"); //  128k mp3
    //  audio.connecttospeech("Wenn die Hunde schlafen, kann der Wolf gut Schafe stehlen.", "de");
    // 音频播放循环
    // String current_dir = "/";

    File root = SD.open("/");
    File entry = root.openNextFile();
    while (1)
    {
        while (!entry)
        {
            root.rewindDirectory();
            entry = root.openNextFile();
        }
        if (!entry.isDirectory() && String(entry.name()).endsWith(".mp3"))
        {
            Serial.printf("Playing %s\n", entry.name());
            songname = entry.name();
            lrcFileName = "/" + String(entry.name()).substring(0, String(entry.name()).lastIndexOf(".")) + ".lrc";
            Serial.printf("LRC FILE: %s\n", lrcFileName.c_str());

            displayImageFromMP3(entry);

            // size_t imagePosition, imageSize;
            // if (findImageInMP3(entry, imagePosition, imageSize))
            // {
            //     Serial.printf("Image found at position: %u, size: %u\n", imagePosition, imageSize);

            //     // Now you can use the audio_id3image function to extract and display the image
            //     audio_id3image(entry, imagePosition, imageSize);
            // }

            //display_image_with_lvgl("S:cover.jpg");

            Serial.println("printLyricsTask Begin.");
            readAndParseLRCFile(lrcFileName.c_str());
            if (readFlag)
            {
                for (int i = 0; i < lyricsCount; i++)
                {
                    Serial.printf("[%d s] %s\n", timestamps[i], lyrics[i]);
                }
                Serial.println("readAndParseLRCFile completed.");
            }

            if (!audio.connecttoFS(SD, songname.c_str()))
            {
                Serial.printf("Failed to play %s\n", songname);
                entry = root.openNextFile();

                vTaskDelay(1000 / portTICK_PERIOD_MS); // 失败后稍等片刻再试
                continue;
            }
            else
            {
                // root.close();
                // entry.close();
                // displayImageFromMP3(entry);

                /*
                String lrcContent = readLRCFile(lrcFileName.c_str());
                if (lrcContent.length() > 0)
                {
                    parseLRC(lrcContent);
                }*/
                // 打印整个 LRC 内容以确认读取正确
                // Serial.println("LRC File Content:");
                // Serial.println(lrcContent.c_str());
            }

            while (audio.isRunning())
            {                                                      // 播放音频
                int song_totalTime = audio.getAudioFileDuration(); // 获取音频文件的总时长

                // 获取播放进度
                currentTime = audio.getAudioCurrentTime();
                if (song_totalTime > 0)
                {
                    int progress = (currentTime * 100) / song_totalTime;
                    Serial.printf("Progress: %d%% [%d/%d]\n", progress, currentTime, song_totalTime);
                }

                for (int i = 0; i < lyricsCount; i++)
                {
                    if (currentTime == timestamps[i])
                    {
                        Serial.println(lyrics[i]);
                        if (!page0_flag)
                        {
                            lv_label_set_text(label_LRC, lyrics[i]);
                        }
                    }
                }

                vTaskDelay(1000); // 每秒更新一次进度
            }

            Serial.printf("Finished playing %s\n", songname);
            readFlag = false;
        }
        if (!page0_flag)
        {
            lv_label_set_text(label_LRC, "");
        }
        freeLyricsData();
        entry = root.openNextFile();
        audio.stopSong();
    }
    vTaskDelete(NULL);
}