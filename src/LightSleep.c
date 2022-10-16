#include <stdio.h>
#include "esp_sleep.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "driver/uart.h"
#include <sys/time.h>
#include <time.h>

static const char *TAG = "SleepClient";
    
void lightSleep()
{
    esp_err_t ret;

    esp_sleep_enable_timer_wakeup(8000000);
    ESP_LOGI(TAG, "Entering light sleep");

    esp_wifi_stop();
    /* To make sure the complete line is printed before entering sleep mode,
        * need to wait until UART TX FIFO is empty:
        */
    uart_wait_tx_idle_polling(CONFIG_ESP_CONSOLE_UART_NUM);

    /* Get timestamp before entering sleep */
    int64_t t_before_us = esp_timer_get_time();
    
    /* Enter sleep mode */
    esp_light_sleep_start();
    /* Execution continues here after wakeup */
    /* Get timestamp after waking up from sleep */
    int64_t t_after_us = esp_timer_get_time();

    /* Determine wake up reason */
    const char* wakeup_reason;
    switch (esp_sleep_get_wakeup_cause()) {
        case ESP_SLEEP_WAKEUP_TIMER:
            wakeup_reason = "timer";
            break;
        case ESP_SLEEP_WAKEUP_GPIO:
            wakeup_reason = "pin";
            break;
        case ESP_SLEEP_WAKEUP_UART:
            wakeup_reason = "UART";
            break;
        default:
            wakeup_reason = "other";
            break;
    }
    ESP_LOGI(TAG, "Returned from light sleep, reason: %s, t=%lld ms, slept for %lld ms",
            wakeup_reason, t_after_us / 1000, (t_after_us - t_before_us) / 1000);

    if ((ret = esp_wifi_start()) != ESP_OK) {
        ESP_LOGE(TAG, "WiFi Start error: %d", ret);
    }
}