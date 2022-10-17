/* UART asynchronous example, that uses separate RX and TX tasks

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"
#include "esp_pm.h"
#include "esp_err.h"
#include "esp_sleep.h"

extern void lightSleep();

static const int RX_BUF_SIZE = 1024;
static int uart_num;

#define TXD_PIN (GPIO_NUM_4)
#define RXD_PIN (GPIO_NUM_5)


void UART_init(int uartNum) {
    uart_num = uartNum;
    const uart_config_t uart_config = {
        .baud_rate = 2400,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    // We won't use a buffer for sending data.
    uart_driver_install(uart_num, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(uart_num, &uart_config);
    uart_set_pin(uart_num, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

int sendData(const char* logName, const char* data)
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(uart_num, data, len);
    ESP_LOGI(logName, "Wrote %d bytes", txBytes);
    return txBytes;
}

void tx_task(void *arg)
{
    static const char *TX_TASK_TAG = "TX_TASK";

    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
    while (1) {
        sendData(TX_TASK_TAG, "Hello world");
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

void rx_task(void *arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    static int64_t t_before_us = 0;
    static int64_t t_after_us = 0;
    uint64_t lightSleepTime = 8000000;
    int rxBytes = 0;

    esp_log_level_set(RX_TASK_TAG, ESP_LOG_DEBUG);

    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    while (1) {
        rxBytes = uart_read_bytes(uart_num, data, RX_BUF_SIZE, pdMS_TO_TICKS(500));        
        if (rxBytes > 0) {
            data[rxBytes] = 0;
            ESP_LOGD(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, data);
            ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);

            /* Get timestamp before processing */
            t_before_us = esp_timer_get_time();

            // TO DO: decrypt and upload Kamstrup data
            vTaskDelay(pdMS_TO_TICKS(1000));

            // Adjust time of light-sleep
            /* Get timestamp after processing */
            t_after_us = esp_timer_get_time();
            lightSleepTime = 8000000 - (t_after_us - t_before_us);
            ESP_LOGI(RX_TASK_TAG, "Entering light sleep for: %lld uS Diff: %lld uS", lightSleepTime, (t_after_us - t_before_us));
            esp_sleep_enable_timer_wakeup(lightSleepTime);

            // Test needs to use vTaskDelay as light-sleep stops the TX task
            //vTaskDelay(pdMS_TO_TICKS(lightSleepTime / 1000));
            lightSleep();
            ESP_LOGI(RX_TASK_TAG, "Exiting light sleep!");
        } else {
            if (rxBytes < 0) {
                ESP_LOGE(RX_TASK_TAG, "Error from 'uart_read_bytes': %d", rxBytes);
            } else {
                if(rxBytes == 0) {
                    ESP_LOGD(RX_TASK_TAG, "No bytes received!");
                }
            }
        }
    }
    free(data);
}
