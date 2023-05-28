#include "freertos/FreeRTOS.h"
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/event_groups.h>
#include <esp_log.h>
#include <string.h>
#include <unistd.h>
#include "driver/gpio.h"
#include <esp_timer.h>

// The Gas input
#define GasOn GPIO_NUM_34
#define GAS_ON 10
#define GAS_OFF 0

// The error LED
#define ERROR_LED GPIO_NUM_2
#define LED_ON 1
#define LED_OFF 0

//static const char *TAG = "GasSensor";

extern SemaphoreHandle_t mtexCurrentGas;
extern SemaphoreHandle_t mtexCurrentDuty;

QueueHandle_t interputQueue;
static int CurrentGas = GAS_OFF;
static float dutyCycle = 0.0;

static void IRAM_ATTR gpio_interrupt_handler(void *args)
{
    int pinNumber = (int)args;
    xQueueSendFromISR(interputQueue, &pinNumber, NULL);
}

// Helper function, returns flame on/off status (0 = Off, 10 = On)
void GetCurrentGas(int *Gas){
	xSemaphoreTake(mtexCurrentGas, portMAX_DELAY );
	{
		*Gas = CurrentGas;
	}
	xSemaphoreGive(mtexCurrentGas);	
}

// Helper function, returns the flame on duty cycle i %
void GetCurrentDuty(float *duty){
	xSemaphoreTake(mtexCurrentDuty, portMAX_DELAY );
	{
		*duty = dutyCycle;
	}
	xSemaphoreGive(mtexCurrentDuty);	
}

// GetGasTask() reads continuously (every 10 mS) the optic gas on sensor 
void GetGasOnTask(void *data)
{
	bool FlameOn = false;
	int64_t timerold;
	int32_t OnTime = 0, OffTime = 0, OnTimeOld;
	int pinNumber;
	float flameOnAvg[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
	int ix = 0;
	float tmp = 0.0;

	// Create the ISR queue
    interputQueue = xQueueCreate(10, sizeof(int));

	// Install the ISR
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GasOn, gpio_interrupt_handler, (void *)GasOn);

	// Initiate timers
	timerold = esp_timer_get_time();
	OnTimeOld = xTaskGetTickCount();

	// The Gas ON/OFF data collector task.
	// The flame on LED pulsrate with a cycle time of ~23 ms, being on in ~6 ms.
	// Negative flanks (LED off) triggers the ISR and the cycle timer (timerold) is reset.
	// As long as the cycle timer is not > 35 ms the FlameOn signal is true.
	while (1) {
		// Any interrupt within 10 ms
		if (xQueueReceive(interputQueue, &pinNumber, pdMS_TO_TICKS(10)) == pdTRUE) {
			FlameOn = true;
			// Reset cycle timer
			timerold = esp_timer_get_time();
			// Update Flame on status
			xSemaphoreTake(mtexCurrentGas, portMAX_DELAY );
				CurrentGas = GAS_ON;
			xSemaphoreGive(mtexCurrentGas);

			gpio_set_level(ERROR_LED, LED_ON);
		}
		else {
			// Cycle timer > 35 ms (no interrupts within 35 ms)
			if ((esp_timer_get_time() - timerold) > 35000) {
				// FlameOn state > 5 sec. (filter out any flameOn < 5 sec.)
				if (FlameOn && (OnTime > pdMS_TO_TICKS(5000))) {
					// Running average over 5 measurements
					tmp = (float) OnTime / (OnTime + OffTime);
					flameOnAvg[ix] = tmp;
					// Circular buffer
					ix++;
					if (ix > 4) {
						ix = 0;
					}

					// Average
					tmp = 0.0;
					for (int i = 0; i < 5; i++) {
						tmp += flameOnAvg[i];
					}
					tmp = tmp / 5;

					// Update Flame on duty cycle (%)
					xSemaphoreTake(mtexCurrentDuty, portMAX_DELAY );
						dutyCycle = tmp * 100;
					xSemaphoreGive(mtexCurrentDuty);
					ESP_LOGI("Gas", "AVG: %.1f IX: %d", tmp * 100, ix);
					// Cleanup for next flame on duty cycle
					OnTime = 0;
					OffTime = 0;
				}
				// Flame off
				FlameOn = false;
				// Update Flame on status
				xSemaphoreTake(mtexCurrentGas, portMAX_DELAY );
					CurrentGas = GAS_OFF;
				xSemaphoreGive(mtexCurrentGas);

				gpio_set_level(ERROR_LED, LED_OFF);
			}
		}
		// Measure the time for flame on and off
		if (FlameOn) {
			OnTime += (int32_t) xTaskGetTickCount() - OnTimeOld;
		}
		else {
			OffTime += (int32_t) xTaskGetTickCount() - OnTimeOld;
		}
		OnTimeOld = xTaskGetTickCount();
	}
}