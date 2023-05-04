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
#include "mbusparser.h"
#include "esp_timer.h"

// Encrypted test message
/* 
unsigned char cipher_text[] = {0x7E, 0xA1, 0xE9, 0x41, 0x03, 0x13, 0xC6, 0x37, 0xE6, 0xE7, 0x00, 0xDB, 0x08, 0x4B, 0x41, 0x4D, 0x45, 0x01, 0xA4, 0xDC, 0x52, 0x82, 0x01, 0xD0, 0x30, 0x00, 0x07, 0x88, 0xE1, 0xA0, 0x39, 0xB2, 0xD1, 0x4C, 0x71, 0x2D, 0xD4, 0xD8, 0xC8, 0x44, 0x0D, 0x53, 0x68,
                               0xE4, 0x33, 0xBD, 0x70, 0xB7, 0x36, 0x81, 0xE9, 0xA9, 0xEF, 0xFE, 0x38, 0xF1, 0x75, 0xA3, 0x7D, 0xE9, 0xCD, 0xE6, 0x4E, 0x8F, 0x78, 0x0D, 0x8F, 0x18, 0xB4, 0x3F, 0xC0, 0x59, 0xD8, 0x79, 0x02,
                               0xF3, 0xD7, 0x47, 0xB8, 0x14, 0xBC, 0xD0, 0x6A, 0x47, 0x00, 0x68, 0x78, 0x01, 0xBD, 0x5D, 0x06, 0x61, 0x20, 0x54, 0x50, 0x7D, 0x44, 0xE7, 0x66, 0x98, 0xCC, 0x3E, 0x35, 0xCC, 0x9D, 0xE6, 0x2C,
                               0x28, 0x4C, 0x0D, 0xEE, 0xA6, 0x35, 0xB9, 0xBF, 0xC5, 0x6C, 0xE1, 0xFE, 0x5A, 0x3A, 0x1E, 0x5E, 0x27, 0x0B, 0x0C, 0x18, 0x1A, 0xCF, 0x02, 0x15, 0x1F, 0xCC, 0x59, 0x21, 0x34, 0xD0, 0x4F, 0x02,
                               0x92, 0xB5, 0xA3, 0x53, 0x38, 0xD7, 0xB7, 0x81, 0xBB, 0x1F, 0x2A, 0x7E, 0x40, 0x71, 0x81, 0x5C, 0xEE, 0xD5, 0xD4, 0xBA, 0xC6, 0xEF, 0xAA, 0xAF, 0xF9, 0x79, 0xEF, 0x96, 0x9D, 0x0D, 0xB4, 0x6F,
                               0x51, 0xE5, 0xE0, 0xFC, 0x00, 0xF5, 0xAD, 0x10, 0x5F, 0xBE, 0xF9, 0x5F, 0xC5, 0xF8, 0x85, 0x46, 0x0B, 0x56, 0x32, 0x55, 0x4A, 0xC0, 0x5D, 0x9E, 0xB4, 0xF6, 0x5F, 0xF2, 0x23, 0x97, 0x2A, 0x47,
                               0xCF, 0xD4, 0x34, 0xB5, 0xF5, 0xE2, 0xD8, 0x53, 0xEA, 0x4C, 0x14, 0x72, 0x75, 0x86, 0xF0, 0xE2, 0x1C, 0x6E, 0xE1, 0x25, 0x26, 0x8C, 0xB4, 0xDC, 0x7E, 0xC5, 0xB1, 0x0F, 0x84, 0x83, 0xC0, 0x10,
                               0xC8, 0xE6, 0x88, 0xDF, 0x86, 0x58, 0x4D, 0x7C, 0x29, 0xD8, 0x17, 0x31, 0xA4, 0xE0, 0x96, 0x91, 0x41, 0xB6, 0xAD, 0xD9, 0x42, 0xE4, 0x0A, 0x96, 0xE3, 0xE2, 0xDC, 0x2F, 0x90, 0x20, 0xBF, 0x9D,
                               0x58, 0x02, 0xA1, 0x8D, 0xC9, 0x85, 0xBB, 0x54, 0x22, 0xF4, 0x70, 0xC0, 0x62, 0x9D, 0x22, 0xDB, 0x6F, 0x16, 0xB6, 0x64, 0x7D, 0xB3, 0xC9, 0xF7, 0x27, 0xC1, 0x70, 0xC2, 0xDB, 0x4C, 0x9A, 0x23,
                               0x0F, 0x82, 0x83, 0x46, 0xE8, 0x6F, 0x56, 0xD3, 0x47, 0xB6, 0x2B, 0xFE, 0x28, 0xA5, 0x07, 0xAE, 0x2A, 0x21, 0x9A, 0xCC, 0x63, 0xAD, 0x5E, 0xE0, 0x6E, 0xCB, 0x94, 0x9D, 0xC4, 0xFD, 0xC2, 0xD2,
                               0xF8, 0x08, 0x02, 0x7A, 0x4A, 0x4C, 0x67, 0x7C, 0x93, 0xC4, 0xC8, 0x90, 0x9D, 0x73, 0x36, 0xD8, 0xB8, 0xB3, 0x79, 0x18, 0x36, 0xC5, 0x55, 0x5E, 0x74, 0xE2, 0x54, 0x82, 0xA4, 0x4A, 0x5F, 0x6D,
                               0x35, 0x8D, 0xCE, 0x84, 0x50, 0x47, 0x8F, 0x8B, 0x2C, 0x5D, 0x56, 0x66, 0xC2, 0xCF, 0x8B, 0xD1, 0x90, 0xD4, 0x87, 0x4E, 0x70, 0x7F, 0x5D, 0x4B, 0xCD, 0xE9, 0x4E, 0x91, 0x35, 0x5A, 0x81, 0x39,
                               0xC3, 0xE4, 0x1C, 0xD5, 0xF0, 0x88, 0x99, 0xB6, 0x5E, 0xE9, 0xB5, 0x9F, 0xC6, 0x03, 0x72, 0xE4, 0xF9, 0x2B, 0xC9, 0x98, 0x26, 0xB8, 0xC1, 0x47, 0xF2, 0x09, 0x5F, 0xB3, 0x8A, 0x89, 0x14, 0x09,
                               0xAA, 0x81, 0xE2, 0x27, 0x07, 0x6C, 0x21, 0xCB, 0x7C, 0xAF, 0x73, 0xB3, 0xE8, 0xA4, 0xE2, 0x56, 0xC7, 0x0D, 0x95, 0x47, 0xA9, 0x14, 0xF3, 0x9C, 0x16, 0x93, 0x76, 0xBF, 0x92, 0x2B, 0x08, 0x06,
                               0x4E, 0xC4, 0xFB, 0x31, 0xD4, 0x4E, 0xCD, 0x72, 0x1D, 0x1A, 0x15, 0x1E, 0x4E, 0x68, 0x4F, 0x0B, 0x26, 0x85, 0xC4, 0xB6, 0x9D, 0x96, 0xF5, 0xFB, 0x52, 0xD0, 0xB8, 0x12, 0x79, 0x36, 0x24, 0x7E};
*/

extern void lightSleep();
extern int KamDecrypt(unsigned char *cipher_text, int cipher_textLen);
extern int PublishMqtt(struct MeterData md);

static const int RX_BUF_SIZE = 500;
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
    uint64_t lightSleepTime = 6000000;
    int rxBytes = 0;

    esp_log_level_set(RX_TASK_TAG, ESP_LOG_DEBUG);

    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    while (1) {
        rxBytes = uart_read_bytes(uart_num, data, RX_BUF_SIZE, pdMS_TO_TICKS(600));        
        if (rxBytes > 0) {
            data[rxBytes] = 0;
            ESP_LOGI(RX_TASK_TAG, "Read %d bytes", rxBytes);
            //ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);

            /* Get timestamp before processing */
            t_before_us = esp_timer_get_time();

            // Decrypt and upload Kamstrup data
            KamDecrypt(data, rxBytes);

            // Adjust time of light-sleep
            /* Get timestamp after processing */
            t_after_us = esp_timer_get_time();
            lightSleepTime = 6000000 - (t_after_us - t_before_us);
            ESP_LOGI(RX_TASK_TAG, "Entering light sleep for: %lld uS Diff: %lld uS", lightSleepTime, (t_after_us - t_before_us));
            esp_sleep_enable_timer_wakeup(lightSleepTime);

            // During test we need to use vTaskDelay as light-sleep stops the TX task
            // vTaskDelay(pdMS_TO_TICKS(lightSleepTime / 1000));
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
