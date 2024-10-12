/*! @mainpage Guia 2 / Ejercicio 1
 *
 * @section genDesc General Description 
 *
 * La siguiente aplicacion realiza mediciones de distancia a partir de un sensor de ultrasonido, estas mediciones son visualizadas a traves de una pantalla LCD.
 * Enciende leds en distintos rangos si la medida es menor de 10 cm no enciende ningun led, enciende 1 led, 2 leds o 3 leds si la distancia es mayor a 10cm, 20cm y 30 cm respectivamente.
 * Tiene la funcionalidad mediante 2 switchs de detener la medicion (apagando LCD y leds) y de mantener la ultima medida en el lcd.
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
/*==================[macros and definitions]=================================*/

/** @def PERDIOD_TECLAS
 *  @brief Tiempo de espera (ms) para la lectura del estado de botones de la placa
*/
#define CONFIG_BLINK_PERIOD_TECLAS 200


/** @def PERDIOD_MEDICION
 *  @brief Tiempo de espera (ms) para la lectura, escritura de las mediciones en LCD y actualizacion de los LEDS
*/
#define CONFIG_BLINK_PERIOD_MEDICION 1000

/*==================[internal data definition]===============================*/


/** @def hold 
 * @brief Variable global de tipo booleana que indica si se debe mantener la ultima medicion en LCD */
bool hold = false;

/** @def medir 
 * @brief Variable global de tipo booleana que indica si se realizaran las mediciones */
bool medir = true;

/** @def valor_medicion
 *  @brief Variable global de tipo entero sin signo para registrar los velores medidos  */
uint16_t valor_medicion = 0;

/*==================[internal functions declaration]=========================*/

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
		if (medir == true)
		{
			valor_medicion = HcSr04ReadDistanceInCentimeters();
		}
		vTaskDelay(CONFIG_BLINK_PERIOD_MEDICION / portTICK_PERIOD_MS);
	}
}


/** @fn mostrar(void *pvParameter)
 * @brief Tarea para escribir la distancia en LCD y actualizar el encendido de los leds.
*/
static void mostrar(void *pvParameter)
{

	while (true)
	{
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
		vTaskDelay(CONFIG_BLINK_PERIOD_MEDICION / portTICK_PERIOD_MS);
	}
}

/** @fn Teclas(void *pvParameter){
 * @brief Tarea encargada de leer el estado de los switchs.
*/
static void teclas(void *pvParameter)
{
	uint8_t teclas;
	while (true)
	{
		teclas = SwitchesRead();
		switch (teclas)
		{
		case SWITCH_1:
			medir = !medir;

			break;
		case SWITCH_2:
			hold = !hold;
			break;
		}
		vTaskDelay(CONFIG_BLINK_PERIOD_TECLAS / portTICK_PERIOD_MS);
	}
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
	/* Inicialización de LCD, Leds, Sensor de Ultrasonido (configuracion de pines) y Switchs */
	LcdItsE0803Init();
	LedsInit();
	HcSr04Init(GPIO_3, GPIO_2);
	SwitchesInit();

	/* Creacion de las Tareas medicion, mostrar y teclas*/
	xTaskCreate(&teclas, "teclas", 512, NULL, 5, NULL);
	xTaskCreate(&mostrar, "mostrar", 512, NULL, 5, NULL);
	xTaskCreate(&medicion, "medicion", 512, NULL, 5, NULL);
}
/*==================[end of file]============================================*/