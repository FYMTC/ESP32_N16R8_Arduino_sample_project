#if 0
#include <Arduino.h>
#include <USB.h>
#include "SPIFFS.h"

 #include <inttypes.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/event_groups.h"
// #include "freertos/task.h"
// #include "esp_err.h"
// #include "esp_log.h"
// #include "esp_spiffs.h"
 #include "usb_stream.h"
#include "audio_player.h"

// #include "hal/i2s_types.h"
// #include "hal/i2s_hal.h"
// #include ""
#include "ESP_I2S.h"

#define SPIFFS_BASE "/spiffs"
#define MP3_FILE_NAME "/new_epic.mp3"
#define BIT1_SPK_START (0x01 << 0)
static const char *TAG = "usb_audio_player";
static EventGroupHandle_t s_evt_handle;

static esp_err_t _audio_player_mute_fn(AUDIO_PLAYER_MUTE_SETTING setting)
{
    Serial.printf("mute setting: %s\n", setting == 0 ? "mute" : "unmute");
    return usb_streaming_control(STREAM_UAC_SPK, CTRL_UAC_MUTE, (void *)(setting == 0 ? 1 : 0));
}

static esp_err_t _audio_reconfig_std_clock(uint32_t rate, uint32_t bits_cfg, i2s_slot_mode_t ch)
{
    Serial.printf("Re-config: speaker rate %" PRIu32 ", bits %" PRIu32 ", mode %s\n", rate, bits_cfg, ch == 1 ? "MONO" : (ch == 2 ? "STEREO" : "INVALID"));
    usb_streaming_control(STREAM_UAC_SPK, CTRL_SUSPEND, NULL);
    esp_err_t ret = uac_frame_size_reset(STREAM_UAC_SPK, ch, bits_cfg, rate);
    usb_streaming_control(STREAM_UAC_SPK, CTRL_RESUME, NULL);
    return ret;
}

static esp_err_t _audio_player_write_fn(void *audio_buffer, size_t len, size_t *bytes_written, uint32_t timeout_ms)
{
    *bytes_written = 0;
    esp_err_t ret = uac_spk_streaming_write(audio_buffer, len, timeout_ms);
    if (ret == ESP_OK)
    {
        *bytes_written = len;
    }
    return ret;
}

static void _audio_player_callback(audio_player_cb_ctx_t *ctx)
{
    Serial.printf("ctx->audio_event = %d\n", ctx->audio_event);
    switch (ctx->audio_event)
    {
    case AUDIO_PLAYER_CALLBACK_EVENT_IDLE:
    {
        Serial.println("AUDIO_PLAYER_REQUEST_IDLE");
        usb_streaming_control(STREAM_UAC_SPK, CTRL_SUSPEND, NULL);
        FILE *fp = fopen(SPIFFS_BASE MP3_FILE_NAME, "rb");
        if (fp)
        {
            Serial.printf("Playing '%s'\n", MP3_FILE_NAME);
            audio_player_play(fp);
        }
        else
        {
            Serial.printf("unable to open filename '%s'\n", MP3_FILE_NAME);
        }
        break;
    }
    case AUDIO_PLAYER_CALLBACK_EVENT_PLAYING:
        usb_streaming_control(STREAM_UAC_SPK, CTRL_RESUME, NULL);
        Serial.println("AUDIO_PLAYER_REQUEST_PLAY");
        break;
    case AUDIO_PLAYER_CALLBACK_EVENT_PAUSE:
        Serial.println("AUDIO_PLAYER_REQUEST_PAUSE");
        break;
    default:
        break;
    }
}

static void stream_state_changed_cb(usb_stream_state_t event, void *arg)
{
    switch (event)
    {
    case STREAM_CONNECTED:
    {
        size_t frame_size = 0;
        size_t frame_index = 0;
        uac_frame_size_list_get(STREAM_UAC_SPK, NULL, &frame_size, &frame_index);
        if (frame_size)
        {
            Serial.printf("UAC SPK: get frame list size = %u, current = %u\n", frame_size, frame_index);
            uac_frame_size_t *spk_frame_list = (uac_frame_size_t *)malloc(frame_size * sizeof(uac_frame_size_t));
            uac_frame_size_list_get(STREAM_UAC_SPK, spk_frame_list, NULL, NULL);
            for (size_t i = 0; i < frame_size; i++)
            {
                Serial.printf("\t [%u] ch_num = %u, bit_resolution = %u, samples_frequence = %" PRIu32 ", samples_frequence_min = %" PRIu32 ", samples_frequence_max = %" PRIu32 "\n",
                              i, spk_frame_list[i].ch_num, spk_frame_list[i].bit_resolution, spk_frame_list[i].samples_frequence,
                              spk_frame_list[i].samples_frequence_min, spk_frame_list[i].samples_frequence_max);
            }
            Serial.printf("UAC SPK: use frame[%u] ch_num = %u, bit_resolution = %u, samples_frequence = %" PRIu32 "\n",
                          frame_index, spk_frame_list[frame_index].ch_num, spk_frame_list[frame_index].bit_resolution, spk_frame_list[frame_index].samples_frequence);
            usb_streaming_control(STREAM_UAC_SPK, CTRL_UAC_VOLUME, (void *)45);
            if (xEventGroupGetBits(s_evt_handle) & BIT1_SPK_START)
            {
                audio_player_resume();
            }
            else
            {
                xEventGroupSetBits(s_evt_handle, BIT1_SPK_START);
            }
        }
        else
        {
            Serial.printf("UAC SPK: get frame list size = %u\n", frame_size);
        }
        Serial.println("Device connected");
        break;
    }
    case STREAM_DISCONNECTED:
        audio_player_pause();
        Serial.println("Device disconnected");
        break;
    default:
        Serial.println("Unknown event");
        break;
    }
}

void USB_task(void *pvParam)
{
    Serial.begin(115200);
    s_evt_handle = xEventGroupCreate();
    assert(s_evt_handle != NULL);

    if (!SPIFFS.begin(true))
    {
        Serial.println("SPIFFS Mount Failed");
        return;
    }

    uac_config_t uac_config = {
        .spk_ch_num = UAC_CH_ANY,
        .spk_bit_resolution = UAC_BITS_ANY,
        .spk_samples_frequence = UAC_FREQUENCY_ANY,
        .spk_buf_size = 96000,
        .flags = FLAG_UAC_SPK_SUSPEND_AFTER_START};
    ESP_ERROR_CHECK(uac_streaming_config(&uac_config));
    ESP_ERROR_CHECK(usb_streaming_state_register(&stream_state_changed_cb, NULL));
    ESP_ERROR_CHECK(usb_streaming_start());
    ESP_ERROR_CHECK(usb_streaming_connect_wait(portMAX_DELAY));
    xEventGroupWaitBits(s_evt_handle, BIT1_SPK_START, pdTRUE, false, portMAX_DELAY);

    audio_player_config_t config = {.mute_fn = _audio_player_mute_fn,
                                    .write_fn = _audio_player_write_fn,
                                    .clk_set_fn = _audio_reconfig_std_clock,
                                    .priority = 1};
    ESP_ERROR_CHECK(audio_player_new(config));
    ESP_ERROR_CHECK(audio_player_callback_register(_audio_player_callback, NULL));

    FILE *fp = fopen(SPIFFS_BASE MP3_FILE_NAME, "rb");
    if (fp)
    {
        Serial.printf("Playing '%s'\n", MP3_FILE_NAME);
        audio_player_play(fp);
    }
    else
    {
        Serial.printf("unable to open filename '%s'\n", MP3_FILE_NAME);
    }

    for (;;)
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

#endif