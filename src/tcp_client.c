/* BSD Socket API Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "addr_from_stdin.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "driver/gpio.h"

extern void GetTempTask(void *data);
extern void PublishMqttTask(void *pvParameters);

// Semaphor when getting the current temperature
SemaphoreHandle_t mtexCurrentTemp = NULL;

// The error LED
#define ERROR_LED GPIO_NUM_2
#define LED_ON 1
#define LED_OFF 0

// The FAN output
#define FAN GPIO_NUM_4
#define FAN_ON 0
#define FAN_OFF 1

void app_main(void)
{
    // Configure the Fan to GPIO #4 and ERROR LED to GPIO #2
    gpio_config_t IOpin = {
                            .mode = GPIO_MODE_OUTPUT,
                            .pin_bit_mask = (1ULL<<FAN) | (1ULL<<ERROR_LED),
                            .pull_down_en = 0,
                            .pull_up_en = 0,
                            .intr_type = GPIO_INTR_DISABLE,
                          };
    // Make sure the FAN is off and the ERROR LED is on before the GPIO is configured
    gpio_set_level(FAN, FAN_OFF);
    gpio_set_level(ERROR_LED, LED_ON);
    gpio_config(&IOpin);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
	configASSERT(mtexCurrentTemp = xSemaphoreCreateMutex());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    xTaskCreate(GetTempTask, "Temperature", 3000, NULL, 5, NULL);
    xTaskCreate(PublishMqttTask, "mqtt_client", 4096, NULL, 3, NULL);

    gpio_set_level(ERROR_LED, LED_OFF);
}
