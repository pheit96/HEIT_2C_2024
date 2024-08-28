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

/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 100
#define ON 1
#define OFF 2
#define TOGGLE 3
/*==================[internal data definition]===============================*/

struct leds {
	uint8_t mode;
	uint8_t n_led;
	uint8_t n_ciclos;
	uint16_t periodo; 
			} my_leds;

/*==================[internal functions declaration]=========================*/
			void control_leds(struct leds *my_leds) {
				uint8_t i = 0;
				LedsInit();

				if (my_leds->mode == ON)
				{
					if (my_leds->n_led == 1)
					{
						LedOn(LED_1);
					}
					else if (my_leds->n_led == 2)
					{
						LedOn(LED_2);
					}
					else if (my_leds->n_led == 3)
					{
						LedOn(LED_3);
					}
				}
				else if (my_leds->mode == OFF)
				{
					if (my_leds->n_led == 1)
					{
						LedOff(LED_1);
					}
					else if (my_leds->n_led == 2)
					{
						LedOff(LED_2);
					}
				
					else if (my_leds->n_led == 3)
					{
					LedOff(LED_3);
					}
				}
				else if (my_leds->mode == TOGGLE)
				{
					while (i < my_leds->n_ciclos)
					{

						if (my_leds->n_led == 1)
						{
							LedToggle(LED_1);
						}
						else if (my_leds->n_led == 2)
						{
							LedToggle(LED_2);
						}
						else if (my_leds->n_led == 3)
						{
							LedToggle(LED_3);
						}
						i++;

						for (uint8_t j = 0; j < my_leds->periodo / CONFIG_BLINK_PERIOD; j++)
						{
							vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
						}
					}
				}
			}

/*==================[external functions definition]==========================*/
void app_main(void){

struct leds my_leds = {TOGGLE, 1, 10, 500};
control_leds(&my_leds);

}
/*==================[end of file]============================================*/