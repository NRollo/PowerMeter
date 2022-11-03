/* Light sleep example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "addr_from_stdin.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "esp_wifi.h"

extern void wifi_init_sta(void);
extern void rx_task (void *arg);
extern void tx_task(void *arg);
extern void UART_init(int uartNum);
extern void mqtt_app_start(void);

void app_main(void)
{
    esp_log_level_set("wifi", ESP_LOG_ERROR);
    esp_log_level_set("wifi station", ESP_LOG_ERROR);
    esp_log_level_set("esp_netif_handlers", ESP_LOG_WARN);
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init_sta();
    UART_init(UART_NUM_1);
    mqtt_app_start();

    xTaskCreate(rx_task, "uart_rx_task", 1024*5, NULL, 5, NULL);
    
    // The TX task is used during testing
    //xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, 4, NULL);

    vTaskDelay(pdMS_TO_TICKS(5000));
}
