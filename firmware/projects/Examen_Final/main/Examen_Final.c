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
#include "led.h"
#include "uart_mcu.h"
#include "gpio_mcu.h"
#include "timer_mcu.h"
#include "analog_io_mcu.h"
#include "hc_sr04.h"
/*==================[macros and definitions]=================================*/
#define PERIODO_CONTROL_AGUA_ALIMENTO 200

TaskHandle_t medir_agua_task_handle = NULL;
TaskHandle_t medir_comida_task_handle = NULL;

/*==================[internal data definition]===============================*/
float cantidad_de_agua=0;
float volumen_agua=0;
float peso_comida=0;
uint8_t teclas=0;

/*==================[internal functions declaration]=========================*/

static void medicion(void *pvParameter)
{

	while (true)
	{

		volumen_agua= HcSr04ReadDistanceInCentimeters();
		cantidad_de_agua=(30-volumen_agua)*10*10*3.14;
		if(volumen_agua>22){
			GPIOOn(GPIO_20);
		}
		else{
			GPIOOff(GPIO_20);
		}
		UartSendString(UART_PC,"Agua: ");
		UartSendString(UART_PC,(char *)UartItoa(cantidad_de_agua,10));
		UartSendString(UART_PC," cm3, ");


		vTaskDelay(PERIODO_CONTROL_AGUA_ALIMENTO/ portTICK_PERIOD_MS);
	}
}

static void medir_comida(void *pvParameter){
	uint16_t valor_galga=0;

	while(true){
	
	AnalogInputReadSingle(CH1, &valor_galga);

	peso_comida=valor_galga*1000/3300;

	if(peso_comida<50){
		GPIOOn(GPIO_21);
	}
	else {
		if(peso_comida==500){
			GPIOOff(GPIO_21)
		}
	}
	
	UartSendString(UART_PC,"Alimento: ");
	UartSendString(UART_PC,(char *)UartItoa(peso_comida,10));
	UartSendString(UART_PC," gr.\r\n");
	vTaskDelay(PERIODO_CONTROL_AGUA_ALIMENTO/ portTICK_PERIOD_MS);
	}
}

void Leer_teclas()
{
	UartReadByte(UART_PC, &teclas);
	if (teclas == '1')
	{
		GPIOOn(GPIO_20);
		UartSendString(UART_PC,"Barrera Abierta.\r\n");

	}
	
}
/*==================[external functions definition]==========================*/
void app_main(void){


	HcSr04Init(GPIO_3, GPIO_2);
	LedsInit();
	GPIOInit(GPIO_20, GPIO_OUTPUT);
	GPIOInit(GPIO_21, GPIO_OUTPUT);

	serial_config_t conf_puerto = {
		.port = UART_PC,
		.baud_rate = 9600,
		.func_p = NULL,
		.param_p = NULL,
	};
	UartInit(&conf_puerto);

	analog_input_config_t conversorAD = {
		.input = CH1,		/*!< Inputs: CH0, CH1, CH2, CH3 */
		.mode = ADC_SINGLE, /*!< Mode: single read or continuous read */
		.func_p = Leer_teclas(),		/*!< Pointer to callback function for convertion end (only for continuous mode) */
		.param_p = NULL,	/*!< Pointer to callback function parameters (only for continuous mode) */
		.sample_frec = 0};

	AnalogInputInit(&conversorAD);

xTaskCreate(&medicion,"Medicion",512,NULL,5,&medir_agua_task_handle);
xTaskCreate(&medir_comida,"Control de la cantidad de comida",512,NULL,5,&medir_comida_task_handle);

}
/*==================[end of file]============================================*/