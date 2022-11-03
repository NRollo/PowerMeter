#include <stdio.h>
#include "esp_sleep.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "driver/uart.h"
#include <sys/time.h>
#include <time.h>
#include "mqtt_client.h"

static const char *TAG = "SleepHandler";
extern esp_mqtt_client_handle_t MqttClient;
    
void lightSleep()
{
    esp_err_t ret;

    // Stop the wifi and mqtt client before entering light-sleep
    esp_mqtt_client_stop(MqttClient);
    esp_wifi_stop();

    /* To make sure the complete line is printed before entering sleep mode,
    *  need to wait until UART TX FIFO is empty:
    */
    uart_wait_tx_idle_polling(CONFIG_ESP_CONSOLE_UART_NUM);
    
    /* Enter sleep mode */
    esp_light_sleep_start();
    /* Execution continues here after wakeup */

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
    ESP_LOGD(TAG, "Returned from light sleep, reason: %s", wakeup_reason);

    if ((ret = esp_wifi_start()) != ESP_OK) {
        ESP_LOGE(TAG, "WiFi Start error: %d", ret);
    }
    // Wait for the wifi to start after light sleep
    vTaskDelay(pdMS_TO_TICKS(2000));
    esp_mqtt_client_start(MqttClient);
}