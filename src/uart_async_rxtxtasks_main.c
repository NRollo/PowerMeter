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
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void rx_task(void *arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    int rxBytes = 0;
    int uartWait = 15000;

    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);

    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    while (1) {
        rxBytes = uart_read_bytes(uart_num, data, RX_BUF_SIZE, pdMS_TO_TICKS(uartWait));        
        if (rxBytes > 0) {
            data[rxBytes] = 0;
            ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, data);
            ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);
            uartWait = 3000;
        }
        else {
            if (rxBytes == 0) {
                lightSleep();
            }
            else {
                if (rxBytes < 0) {
                    ESP_LOGE(RX_TASK_TAG, "Error from 'uart_read_bytes': %d", rxBytes);
                }
            }
        }
    }
    free(data);
}
