/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Se diseña un control de agua y alimento para mascotas basado en la ESP-EDU el cual mide el nivel de agua a partir de un sensor de ultrasonido y la cantidad de alimento contenido en el plato a partir de una galga analogica.
 *
 * @section hardConn Hardware Connection
 *
 * |    galga  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	medicion analogica	 	| 	CH1		|
 * | 	+5V  	 	| 	+5V	    	|
 * | 	GND 	 	| 	GND	    	|
 * 
 * |    electrovalvulas |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	AGUA	 	| 	GPIO20		|
 * | 	ALIMENTO  	 	| 	GPIO21	    	|
 * 
 * 
 * ||    HC-SR04     |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	ECHO	 	| 	GPIO_3		|
 * | 	TRIGGER	 	| 	GPIO_2		|
 * | 	+5V  	 	| 	+5V	    	|
 * | 	GND 	 	| 	GND	    	|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Pedro Heit (Pedren83@gmail.com)
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
/** @def PERIODO_CONTROL_AGUA_ALIMENTO
 *  @brief Periodo de delay para controlar el nivel de agua y la cantidad de alimento que se tiene
*/
#define PERIODO_CONTROL_AGUA_ALIMENTO 200

/** @def cantidad_de_agua
 *  @brief Variable global de tipo flotante para registrar el volumen de agua contenido en el recipiente*/
float cantidad_de_agua=0;

/** @def volumen_agua
 *  @brief Variable global de tipo flotante para registrar la distancia desde el ultrasonido hasta la superficie del agua en el recipiente*/
float volumen_agua=0;

/** @def peso_comida
 *  @brief Variable global de tipo flotante para registrar el peso de alimento en el plato*/
float peso_comida=0;

/** @def teclas
 *  @brief Variable global de tipo entero sin signo de 8 bits para leer las teclas ingresadas por puerto serie UART*/
uint8_t teclas=0;

/** @def iniciar
 *  @brief Variable global de tipo booleana que da el estado de funcionamiento del sistema*/
bool iniciar=1;

/*==================[internal data definition]===============================*/

/** @def medir_agua_task_handle
 *  @brief Objeto de tipo TaskHandle_t que se asocia con la tarea medicion*/
TaskHandle_t medir_agua_task_handle = NULL;

/** @def medir_comida_task_handle
 *  @brief Objeto de tipo TaskHandle_t que se asocia con la tarea medir comida*/
TaskHandle_t medir_comida_task_handle = NULL;

/*==================[internal functions declaration]=========================*/

/** @fn medicion(void *pvParameter)
 * @brief Tarea encargada de medir la cantidad de agua que se registra en el recipiente.
*/
static void medicion(void *pvParameter)
{

	while (iniciar)
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


/** @fn medir_comida(void *pvParameter)
 * @brief Tarea encargada de pesar la comida contenida en el plato sobre la balanza.
*/
static void medir_comida(void *pvParameter){
	uint16_t valor_galga=0;

	while(iniciar){
	
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

/** @fn Leer_teclas(void *pvParameter)
 * @brief Lee los valores enviados por puerto serie UART e inicia o detiene el sistema de control de comida .
*/
void Leer_teclas()
{
	UartReadByte(UART_PC, &teclas);
	if (teclas == '1')
	{
		iniciar= !iniciar;
	}
	if(iniciar==true){
		UartSendString(UART_PC,"Sistema funcionando.\r\n");
		LedOn(LED_1);
	}
	else{
		if(iniciar==false){
			UartSendString(UART_PC,"Sistema desactivado.\r\n");
			LedOff(LED_1);
		}
	}
}
/*==================[external functions definition]==========================*/
void app_main(void){

/* Inicialización de Leds, Sensor de Ultrasonido, configuro entradas analogicas y configuro el puerto serie UART*/
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

/* Creo las tareas*/
xTaskCreate(&medicion,"Medicion",512,NULL,5,&medir_agua_task_handle);
xTaskCreate(&medir_comida,"Control de la cantidad de comida",512,NULL,5,&medir_comida_task_handle);

}
/*==================[end of file]============================================*/