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
 * @author Pedro Heit (pedren83@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
#include "hc_sr04.h"
#include "lcditse0803.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD_TECLAS 200
#define CONFIG_BLINK_PERIOD_MEDICION 100
#define CONFIG_BLINK_PERIOD_MEDICION_US 1000000
/*==================[internal data definition]===============================*/
bool hold = false;
bool medir = true;
uint16_t valor_medicion = 0;
uint8_t teclas = 0;
TaskHandle_t medicion_task_handle = NULL;
TaskHandle_t mostrar_task_handle = NULL;
/*==================[internal functions declaration]=========================*/

void FuncTimerA(void *param)
{
	vTaskNotifyGiveFromISR(medicion_task_handle, pdFALSE);
	vTaskNotifyGiveFromISR(mostrar_task_handle, pdFALSE);
}

void Controlar_Leds(void)
{

	if (valor_medicion < 10)
	{
		LedOff(LED_1);
		LedOff(LED_2);
		LedOff(LED_3);
	}
	else
	{
		if (valor_medicion > 10 && valor_medicion < 20)
		{
			LedOn(LED_1);
			LedOff(LED_2);
			LedOff(LED_3);
		}
		else
		{
			if (valor_medicion > 20 && valor_medicion < 30)
			{
				LedOn(LED_1);
				LedOn(LED_2);
				LedOff(LED_3);
			}
			else
			{
				if (valor_medicion > 30)
				{
					LedOn(LED_1);
					LedOn(LED_2);
					LedOn(LED_3);
				}
			}
		}
	}
}

static void medicion(void *pvParameter)
{

	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (medir == true)
		{
			valor_medicion = HcSr04ReadDistanceInCentimeters();
			UartSendString(UART_PC, (char *)UartItoa(valor_medicion, 10));
			UartSendString(UART_PC, "cm \r\n");
		}
	}
}
static void mostrar(void *pvParameter)
{

	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (medir)
		{
			Controlar_Leds();
			if (!hold)
			{
				LcdItsE0803Write(valor_medicion);
			}
		}
		else
		{
			LcdItsE0803Off();
			LedOff(LED_1);
			LedOff(LED_2);
			LedOff(LED_3);
		}
	}
}

void Interrupcion_tecla_1(void)
{
	medir = !medir;
}

void Interrupcion_tecla_2(void)
{
	hold = !hold;
}

void Leer_teclas()
{
	UartReadByte(UART_PC, &teclas);
	if (teclas == 'O')
	{
		Interrupcion_tecla_1();
	}
	else
	{
		if (teclas == 'H')
		{
			Interrupcion_tecla_2();
		}
	}
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	// creo el timer
	timer_config_t Timer_medicion = {
		.timer = TIMER_A,
		.period = CONFIG_BLINK_PERIOD_MEDICION_US,
		.func_p = FuncTimerA,
		.param_p = NULL};
	TimerInit(&Timer_medicion);
	// Inicializo LCD, Switchs y leds
	LcdItsE0803Init();
	LedsInit();
	HcSr04Init(GPIO_3, GPIO_2);
	serial_config_t conf_puerto = {
		.port = UART_PC,
		.baud_rate = 9600,
		.func_p = &Leer_teclas,
		.param_p = NULL,
	};
	UartInit(&conf_puerto);
	SwitchesInit();
	SwitchActivInt(SWITCH_1, &Interrupcion_tecla_1, NULL);
	SwitchActivInt(SWITCH_2, &Interrupcion_tecla_2, NULL);
	// Creo las Tareas
	xTaskCreate(&mostrar, "mostrar", 512, NULL, 5, &mostrar_task_handle);
	xTaskCreate(&medicion, "medicion", 512, NULL, 5, &medicion_task_handle);
	TimerStart(Timer_medicion.timer);
}
/*==================[end of file]============================================*/