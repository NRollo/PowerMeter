#include "owb.h"
#include "owb_rmt.h"
#include "ds18b20.h"
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/event_groups.h>
#include <esp_log.h>
#include <string.h>
#include <unistd.h>

#define DS18B20_RESOLUTION   (DS18B20_RESOLUTION_12_BIT)
// The error LED
#define ERROR_LED GPIO_NUM_2
#define LED_ON 1
#define LED_OFF 0

static const char *TAG = "TempSensor";

extern SemaphoreHandle_t mtexCurrentTemp;

static float CurrentTemp = 0.0;

// Helper function, returns the current temperature
void GetCurrentTemp(float *temp){
	xSemaphoreTake(mtexCurrentTemp, portMAX_DELAY );
	{
		*temp = CurrentTemp;
	}
	xSemaphoreGive(mtexCurrentTemp);	
}

// Initialises the OneWire bus.
// GetTempTask() reads continuously (every 1000 mS) the DS18B20 temperature sensor
// and stores the result in the semaphore protected float 'CurrentTemp' 
void GetTempTask(void *data)
{
	OneWireBus * owb;
	owb_rmt_driver_info rmt_driver_info;
	owb = owb_rmt_initialize(&rmt_driver_info, 21, RMT_CHANNEL_1, RMT_CHANNEL_0);
	owb_use_crc(owb, true);  // enable CRC check for ROM code
	OneWireBus_ROMCode rom_code;
	char buf[100] = {"\0"};

	// One DS18B20 only
	owb_status status = owb_read_rom(owb, &rom_code);
	if (status == OWB_STATUS_OK)
	{
		char rom_code_s[OWB_ROM_CODE_STRING_LENGTH];

		owb_string_from_rom_code(rom_code, rom_code_s, sizeof(rom_code_s));
		sprintf(buf, "Temperature sensor: 0x%s detected", rom_code_s);
		ESP_LOGI(TAG, "%s", buf);
	}
	else
	{
		sprintf(buf, "Error reading temperatur sensor ID: %d", status);
		ESP_LOGI(TAG, "%s", buf);
	}

	// heap allocation
    DS18B20_Info * ds18b20_info = ds18b20_malloc();
	// only one device on bus
    ds18b20_init_solo(ds18b20_info, owb);
	// enable CRC check on all reads
    ds18b20_use_crc(ds18b20_info, true);
	// Resolution: 12 bit
    ds18b20_set_resolution(ds18b20_info, DS18B20_RESOLUTION);

	ESP_LOGI(TAG, "The temperature data collector task is running on CPU: #%d", xPortGetCoreID());

	// The temp. data collector
	float temp = 0.0;
	DS18B20_ERROR ret = DS18B20_OK;
	int blink = 0;
	while (1) {
		if (ret == DS18B20_OK){
			ret = ds18b20_convert_and_read_temp(ds18b20_info, &temp);
			if (ret == DS18B20_OK){
				xSemaphoreTake(mtexCurrentTemp, portMAX_DELAY );
				{
					CurrentTemp = temp;
				}
				xSemaphoreGive(mtexCurrentTemp);
				//ESP_LOGI(TAG, "Temp task running on CPU: #%d", xPortGetCoreID());	
			}
			else{
				sprintf(buf, "Temp. 'convert and read' DS18B20_ERROR: #%d - Check the connection to the Temp. sensor", ret);			
			}
		}
		else {
			blink = 1 ^ blink;
			gpio_set_level(ERROR_LED, blink);
			ret = ds18b20_read_temp(ds18b20_info, &temp);
			if (ret == DS18B20_OK){
				gpio_set_level(ERROR_LED, LED_OFF);
			}
		}
		// Wait until next data collection
		vTaskDelay(pdMS_TO_TICKS(1000));

		//ESP_LOGI(TAG, "Stack watermark: %d", uxTaskGetStackHighWaterMark(hTaskTemp));
	}
}