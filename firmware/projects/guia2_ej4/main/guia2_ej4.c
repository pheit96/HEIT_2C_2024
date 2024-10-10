/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "analog_io_mcu.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
// #include <projdefs.h>

/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD_MEDICION_US 2000
#define CONFIG_BLINK_PERIOD_ECG_US 4000
#define BUFFER_SIZE 231
/*==================[internal data definition]===============================*/
TaskHandle_t AD_task_handle = NULL;
TaskHandle_t DA_task_handle = NULL;
const char ecg[BUFFER_SIZE] = {
    76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77, 77, 76, 76,
};
/*==================[internal functions declaration]=========================*/
void FuncTimerA(void *param)
{
	vTaskNotifyGiveFromISR(AD_task_handle, pdFALSE);
}

void FuncTimerB(void *param)
{
	vTaskNotifyGiveFromISR(DA_task_handle, pdFALSE);
}

static void ConversorAD_Task(void *pvParameter)
{
	volatile uint16_t valor = 0;
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		AnalogInputReadSingle(CH1, &valor);
		UartSendString(UART_PC, (char *)UartItoa(valor, 10));
		UartSendString(UART_PC, "\r\n");
	}
}

static void ConversorDA_Task(void *pvParameter)
{
	volatile uint8_t i = 0;
	while (true)
	{   
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (i >= BUFFER_SIZE)
			i = 0;
	
		AnalogOutputWrite(ecg[i]);
		i++;
	}
}

/*==================[external functions definition]==========================*/
void app_main(void)
{

	// Configuro e inicializo el timer
	timer_config_t Timer_conversorAD = {
		.timer = TIMER_A,
		.period = CONFIG_BLINK_PERIOD_MEDICION_US,
		.func_p = FuncTimerA,
		.param_p = NULL};
	TimerInit(&Timer_conversorAD);

	timer_config_t Timer_conversorDA = {
		.timer = TIMER_B,
		.period = CONFIG_BLINK_PERIOD_ECG_US,
		.func_p = FuncTimerB,
		.param_p = NULL};
	TimerInit(&Timer_conversorDA);

	// Configuro conversor AD
	analog_input_config_t conversorAD = {
		.input = CH1,		/*!< Inputs: CH0, CH1, CH2, CH3 */
		.mode = ADC_SINGLE, /*!< Mode: single read or continuous read */
		.func_p = NULL,		/*!< Pointer to callback function for convertion end (only for continuous mode) */
		.param_p = NULL,	/*!< Pointer to callback function parameters (only for continuous mode) */
		.sample_frec = 0};
	AnalogInputInit(&conversorAD);
	AnalogOutputInit();
	// Configuro UART
	serial_config_t conf_puerto = {
		.port = UART_PC,
		.baud_rate = 115200,
		.func_p = NULL,
		.param_p = NULL,
	};
	UartInit(&conf_puerto);

	xTaskCreate(&ConversorAD_Task, "convertAD", 1024, NULL, 5, &AD_task_handle);
	xTaskCreate(&ConversorDA_Task, "convertDA", 1024, NULL, 5, &DA_task_handle);

	// Arranco los timers
	TimerStart(Timer_conversorAD.timer);
	TimerStart(Timer_conversorDA.timer);
}
/*==================[end of file]============================================*/