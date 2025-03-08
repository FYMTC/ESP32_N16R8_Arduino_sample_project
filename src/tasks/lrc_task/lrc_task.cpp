#include "tasks/lrc_task/lrc_task.h"
#if 0
extern int currentTime;
extern bool lrc_flag;
extern String lrcFileName;
extern lv_obj_t *label_LRC;
extern bool page0_flag;
bool next_lrc_flag = false;
#define MAX_LINES 100       // Adjust according to the expected maximum number of lines
#define MAX_LINE_LENGTH 128 // Maximum length of each line
extern SemaphoreHandle_t sdCardMutex;
// Global variables
int lyricsCount = 0;
char *text_buffer = nullptr;
int *timestamps = nullptr;
char **lyrics = nullptr;
bool readFlag = false;
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

// FreeRTOS task to print lyrics based on current time
void printLyricsTask(void *pvParameters)
{

    while (1)
    {
        if (lrc_flag)
        {
            Serial.println("printLyricsTask Begin.");
            readAndParseLRCFile(lrcFileName.c_str());
            // Free allocated memory
            if (readFlag)
            {
                for (int i = 0; i < lyricsCount; i++)
                {
                    Serial.printf("[%d s] %s\n", timestamps[i], lyrics[i]);
                }
                Serial.println("readAndParseLRCFile completed.");
                next_lrc_flag = true;
                while (lrc_flag)
                {
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
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                }
                next_lrc_flag = false;
                readFlag = false;
                if (!page0_flag)
                {
                    lv_label_set_text(label_LRC, "");
                }
                freeLyricsData();
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
#endif