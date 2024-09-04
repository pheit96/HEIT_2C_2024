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
#include <portmacro.h>
#include "hc_sr04.h"
#include "lcditse0803.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD_TECLAS 200
#define CONFIG_BLINK_PERIOD_MEDICION 1000
/*==================[internal data definition]===============================*/
bool hold=false;
bool medir=true;
uint16_t valor_medicion=0;
/*==================[internal functions declaration]=========================*/
void Controlar_Leds(void){

if(valor_medicion<10){
	LedOff(LED_1);
	LedOff(LED_2);
	LedOff(LED_3);
}
else{
	if(valor_medicion>10 && valor_medicion<20){
		LedOn(LED_1);
		LedOff(LED_2);
		LedOff(LED_3);
	}
else{
if(valor_medicion>20 && valor_medicion<30){
		LedOn(LED_1);
		LedOn(LED_2);
		LedOff(LED_3);
	}
	else{
	if(valor_medicion>30){
		LedOn(LED_1);
		LedOn(LED_2);
		LedOn(LED_3);
	}
}
}
}

}

static void medicion (void *pvParameter){

	while(true){
		if(medir==true){
			valor_medicion=HcSr04ReadDistanceInCentimeters();
		}
 			vTaskDelay(CONFIG_BLINK_PERIOD_MEDICION / portTICK_PERIOD_MS);
	}
	
}
static void mostrar (void *pvParameter){

while(true){

	if(medir&&hold){
		Controlar_Leds();
	}
	else{
		LcdItsE0803Write(valor_medicion);
		Controlar_Leds();
	}
	if(medir==false){
		LcdItsE0803Off();
		LedOff(LED_1);
	LedOff(LED_2);
	LedOff(LED_3);
	}

 vTaskDelay(CONFIG_BLINK_PERIOD_MEDICION / portTICK_PERIOD_MS);
}

}


static void teclas (void *pvParameter){
	uint8_t teclas;
    while(true)    {
    	teclas  = SwitchesRead(); 
    	switch(teclas){
    		case SWITCH_1:
			medir=!medir;

    		break;
    		case SWITCH_2:
    		hold=!hold;
    		break;
    	}
	    vTaskDelay(CONFIG_BLINK_PERIOD_TECLAS / portTICK_PERIOD_MS);
	}
}
/*==================[external functions definition]==========================*/
void app_main(void){
//Inicializo LCD, Switchs y leds
LcdItsE0803Init();
LedsInit();
HcSr04Init(GPIO_3,GPIO_2);
SwitchesInit();
//Creo las Tareas
	xTaskCreate(&teclas, "teclas", 512, NULL, 5, NULL);
    xTaskCreate(&mostrar, "mostrar", 512, NULL, 5, NULL);
    xTaskCreate(&medicion, "medicion", 512, NULL, 5, NULL);

}
/*==================[end of file]============================================*/