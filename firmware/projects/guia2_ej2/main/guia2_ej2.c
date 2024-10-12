/*! @mainpage Guia 2 / Ejercicio 2
 *
 * @section genDesc General Description 
 *
 * La siguiente aplicacion realiza mediciones de distancia a partir de un sensor de ultrasonido, estas mediciones son visualizadas a traves de una pantalla LCD.
 * Enciende leds en distintos rangos si la medida es menor de 10 cm no enciende ningun led, enciende 1 led, 2 leds o 3 leds si la distancia es mayor a 10cm, 20cm o 30 cm respectivamente.
 * Tiene la funcionalidad mediante interrupciones con 2 switchs de detener la medicion (apagando LCD y leds) y de mantener la ultima medida en el lcd.
 *
 * 
 *
 * @section hardConn Hardware Connection
 *
 * |    HC-SR04     |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	ECHO	 	| 	GPIO_3		|
 * | 	TRIGGER	 	| 	GPIO_2		|
 * | 	+5V  	 	| 	+5V	    	|
 * | 	GND 	 	| 	GND	    	|
 * 
 * |    display LCD |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	D1	     	| 	GPIO_20		|
 * | 	D2	     	| 	GPIO_21  	|
 * | 	D3 	     	| 	GPIO_22  	|
 * | 	D4	     	| 	GPIO_23  	|
 * | 	SEL_1	    | 	GPIO_19  	|
 * | 	SEL_2	    | 	GPIO_18  	|
 * | 	SEL_3     	|  	GPIO_9  	|
 * | 	+5V	     	| 	+5V    		|
 * | 	GND	     	| 	GND    		|
 * 
 * 
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 10/10/2024 | Document creation		                         |
 *
 * @author Pedro Heit (Pedren83@gmail.com)
 *
 /*==================[inclusions]=============================================*/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
//#include <portmacro.h>
#include "hc_sr04.h"
#include "lcditse0803.h"
#include "timer_mcu.h"

/*==================[macros and definitions]=================================*/

/** @def CONFIG_BLINK_PERIOD_MEDICION_US
 *  @brief Periodo del timer (en us)
*/
#define CONFIG_BLINK_PERIOD_MEDICION_US 1000000

/*==================[internal data definition]===============================*/

/** @def hold
 *  @brief Variable global de tipo booleana que indica si se debe mantener la ultima medicion en LCD */
bool hold = false;

/** @def medir 
 * @brief Variable global de tipo booleana que indica si se realizaran las mediciones */
bool medir = true;

/** @def valor_medicion
 *  @brief Variable global de tipo entero sin singno de 8 bits para registrar los velores medidos  */
uint16_t valor_medicion = 0;

/** @def medicion_task_handle
 *  @brief Objeto de tipo TaskHandle_t que se asocia con la tarea medicion*/
TaskHandle_t medicion_task_handle = NULL;

/** @def mostrar_task_handle 
 * @brief Objeto de tipo TaskHandle_t que se asocia con la tarea mostrar*/
TaskHandle_t mostrar_task_handle = NULL;

/*==================[internal functions declaration]=========================*/

/**
 * @fn FuncTimerA(void *param)
 * @param param parametro que no se utiliza
 * @brief envia una notificacion a las tareas medicion y mostrar 
*/
void FuncTimerA(void *param)
{
	vTaskNotifyGiveFromISR(medicion_task_handle, pdFALSE);
	vTaskNotifyGiveFromISR(mostrar_task_handle, pdFALSE);
}



/** @fn Controlar_Leds(void)
 * @brief Funcion encargada de controlar el encendido de los leds.
*/
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

/** @fn medicion(void *pvParameter)
 * @brief Tarea realizada para tomar el valor de la medicion y guardar.
*/
static void medicion(void *pvParameter)
{

	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (medir == true)
		{
			valor_medicion = HcSr04ReadDistanceInCentimeters();
		}
	}
}

/** @fn mostrar(void *pvParameter)
 * @brief Tarea para escribir la distancia en LCD y actualizar el encendido de los leds.
*/
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


/**
 * @fn Interrupcion_tecla_1(void)
 * @brief Cambia el estado de la variable booleana medir
 * 
*/
void Interrupcion_tecla_1(void)
{
	medir = !medir;
}


/**
 * @fn Interrupcion_tecla_2(void)
 * @brief Cambia el estado de la variable booleana hold
 * 
*/
void Interrupcion_tecla_2(void)
{
	hold = !hold;
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	/* Inicialización de timer Timer_medicion */
	timer_config_t Timer_medicion = {
		.timer = TIMER_A,
		.period = CONFIG_BLINK_PERIOD_MEDICION_US,
		.func_p = FuncTimerA,
		.param_p = NULL};
	TimerInit(&Timer_medicion);


	/* Inicialización de LCD, Leds, Sensor de Ultrasonido y Switchs */
	LcdItsE0803Init();
	LedsInit();
	HcSr04Init(GPIO_3, GPIO_2);
	SwitchesInit();
	SwitchActivInt(SWITCH_1, &Interrupcion_tecla_1, NULL);
	SwitchActivInt(SWITCH_2, &Interrupcion_tecla_2, NULL);


	/* Creacion de las Tareas medicion, mostrar y disparo del timer */
	xTaskCreate(&mostrar, "mostrar", 512, NULL, 5, &mostrar_task_handle);
	xTaskCreate(&medicion, "medicion", 512, NULL, 5, &medicion_task_handle);
	TimerStart(Timer_medicion.timer);
}
/*==================[end of file]============================================*/