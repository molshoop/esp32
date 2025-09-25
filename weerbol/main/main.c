#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/rmt_tx.h"
#include "led_strip_encoder.h"

#define LED_GPIO 18            // GPIO voor DIN van WS2812B
#define LED_NUMBER 11          // aantal leds in de bol (pas aan!)
#define RESOLUTION_HZ 10000000 // 10 MHz klok

static rmt_channel_handle_t led_chan = NULL;
static rmt_encoder_handle_t led_encoder = NULL;
static rmt_transmit_config_t tx_config = {
    .loop_count = 0,
};

static uint8_t led_buf[LED_NUMBER * 3]; // buffer (GRB volgorde)

// 🔧 Helper: vul alle leds met één kleur
static void fill_color(uint8_t r, uint8_t g, uint8_t b)
{
    for (int i = 0; i < LED_NUMBER; i++)
    {
        led_buf[i * 3 + 0] = g; // G
        led_buf[i * 3 + 1] = r; // R
        led_buf[i * 3 + 2] = b; // B
    }
    rmt_transmit(led_chan, led_encoder, led_buf, sizeof(led_buf), &tx_config);
    rmt_tx_wait_all_done(led_chan, -1);
}

// 🌞 Zonnig: warm geel/oranje gloed
static void weather_sunny()
{
    fill_color(255, 180, 0);
}

// ☁️ Bewolkt: gedimd grijs
static void weather_cloudy()
{
    fill_color(80, 80, 80);
}

// 🌩️ Onweer: basis donkerblauw met flitsen
static void weather_thunderstorm()
{
    fill_color(0, 0, 40); // donkerblauw
    vTaskDelay(pdMS_TO_TICKS(500));

    for (int i = 0; i < 3; i++)
    {
        fill_color(255, 255, 255); // witte flits
        vTaskDelay(pdMS_TO_TICKS(100));
        fill_color(0, 0, 40);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

// ⭐ Heldere nacht: willekeurige twinkelende sterren
static void weather_night_clear()
{
    fill_color(0, 0, 0); // eerst alles uit

    for (int t = 0; t < 30; t++)
    {
        int star = rand() % LED_NUMBER;
        // maak alles zwart
        for (int i = 0; i < LED_NUMBER; i++)
        {
            led_buf[i * 3 + 0] = 0;
            led_buf[i * 3 + 1] = 0;
            led_buf[i * 3 + 2] = 0;
        }
        // zet 1 willekeurige led als ster (wit/koel)
        led_buf[star * 3 + 0] = 30; // G
        led_buf[star * 3 + 1] = 30; // R
        led_buf[star * 3 + 2] = 60; // B (iets meer blauw → koel wit)
        rmt_transmit(led_chan, led_encoder, led_buf, sizeof(led_buf), &tx_config);
        rmt_tx_wait_all_done(led_chan, -1);
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

// ❄️ Sneeuw: wit pulserend
static void weather_snow()
{
    for (int bright = 20; bright < 200; bright += 20)
    {
        fill_color(bright, bright, 255);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    for (int bright = 200; bright > 20; bright -= 20)
    {
        fill_color(bright, bright, 255);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void app_main(void)
{
    // 1. RMT TX kanaal maken
    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .gpio_num = LED_GPIO,
        .mem_block_symbols = 64,
        .resolution_hz = RESOLUTION_HZ,
        .trans_queue_depth = 4,
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &led_chan));
    ESP_ERROR_CHECK(rmt_enable(led_chan));

    // 2. LED strip encoder maken
    led_strip_encoder_config_t encoder_config = {
        .resolution = RESOLUTION_HZ,
    };
    ESP_ERROR_CHECK(rmt_new_led_strip_encoder(&encoder_config, &led_encoder));

    // 3. Demo-loop
    while (1)
    {
        printf("🌞 Zonnig...\n");
        weather_sunny();
        vTaskDelay(pdMS_TO_TICKS(3000));

        printf("☁️ Bewolkt...\n");
        weather_cloudy();
        vTaskDelay(pdMS_TO_TICKS(3000));

        printf("🌩️ Onweer...\n");
        weather_thunderstorm();
        vTaskDelay(pdMS_TO_TICKS(4000));

        printf("⭐ Heldere nacht...\n");
        weather_night_clear();
        vTaskDelay(pdMS_TO_TICKS(4000));

        printf("❄️ Sneeuw...\n");
        weather_snow();
        vTaskDelay(pdMS_TO_TICKS(4000));
    }
}
