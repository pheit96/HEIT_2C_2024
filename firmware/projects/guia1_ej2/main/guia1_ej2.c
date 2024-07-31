/*! @mainpage guia1 ejercicio 2 
 *
 * \section genDesc General Description
 *
 * al presionar el boton 1 parpadea led 1, con boton 2 parpadea led 2 y al presionar ambos juntos parpadea el led 3.
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 31/07/2024 | Document creation		                         |
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
#include <portmacro.h>
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 50
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){
	uint8_t teclas;
	LedsInit();
	SwitchesInit();
    while(1)    {
    	teclas  = SwitchesRead(); 
    	switch(teclas){
    		case SWITCH_1:
    			LedToggle(LED_1);
    		break;
    		case SWITCH_2:
    			LedToggle(LED_2);
    		break;
			case (SWITCH_2|SWITCH_1):
    			LedToggle(LED_3);
    		break;
    	}
	    vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
	}
}