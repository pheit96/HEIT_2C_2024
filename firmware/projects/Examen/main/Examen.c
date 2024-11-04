/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 *Se desarrolla una alerta para ciclistas basado en la ESP-EDU donde se mide la *distancia de los vehiculos que viene atras con un sensor de ultrasonido, con *alarmas dadas por buzzers, ademas se implementa un acelerometro para *identificar cuando el ciclista sufre una caida.
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	Buzzer  	| 	GPIO_20, GPIO_21		|
 * |	UART		|  TX: GPIO18, RX: GPIO19 |
 * |
 * ||    HC-SR04     |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	ECHO	 	| 	GPIO_3		|
 * | 	TRIGGER	 	| 	GPIO_2		|
 * | 	+5V  	 	| 	+5V	    	|
 * | 	GND 	 	| 	GND	    	|
 * 
 * ||    Acelerometro     |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	puerto X	 | 	GPIO_0		|
 * | 	Puerto Y 	| 	GPIO_1		|
 * | 	Puerto Z 	| 	GPIO_2		|
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
 * @author Pedro heit (pedren83@gmail.com)
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

/*==================[macros and definitions]=================================*/
/** @def CONFIG_BLINK_PERIOD_MEDICION_US
 *  @brief Periodo del timer (en us)
*/
#define CONFIG_BLINK_PERIOD_MUESTREO_DISTANCIA 500000

/** @def CONFIG_BLINK_PERIOD_MEDICION_US
 *  @brief Periodo del timer (en us)
*/
#define CONFIG_BLINK_PERIOD_MUESTREO_PRECAUCION 1000000

/** @def CONFIG_BLINK_PERIOD_MEDICION_US
 *  @brief Periodo del timer (en us)
*/
#define CONFIG_BLINK_PERIOD_MUESTREO_ACELEROMETRO 10000

/*==================[internal data definition]===============================*/

TaskHandle_t medir_distancia_task_handle = NULL;
TaskHandle_t precaucion_task_handle = NULL;
TaskHandle_t acelerometro_task_handle = NULL;
TaskHandle_t peligro_task_handle = NULL;
uint16_t valor_medicion = 0;

/*==================[internal functions declaration]=========================*/

void FuncTimerA(void *param)
{
	vTaskNotifyGiveFromISR(medir_distancia_task_handle, pdFALSE);
	vTaskNotifyGiveFromISR(peligro_task_handle, pdFALSE);
}

void FuncTimerB(void *param)
{
	vTaskNotifyGiveFromISR(precaucion_task_handle, pdFALSE);
}

void FuncTimerC(void *param)
{
	vTaskNotifyGiveFromISR(acelerometro_task_handle, pdFALSE);
}

typedef struct
{
	gpio_t pin; /*!< GPIO pin number */
	io_t dir;	/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

void Controlar_Leds(void)
{

	if (valor_medicion > 500)
	{
		LedOn(LED_1);
		LedOff(LED_2);
		LedOff(LED_3);
	}
	else
	{
		if (valor_medicion > 300 && valor_medicion < 500)
		{
			LedOn(LED_1);
			LedOn(LED_2);
			LedOff(LED_3);
			GPIOToggle(GPIO_20);
			UartSendByte(UART_CONNECTOR, "Precaución, Vehiculo cerca.\r\n");
		}
		else
		{
			if (valor_medicion < 300)
			{
				LedOn(LED_1);
				LedOn(LED_2);
				LedOn(LED_3);
				GPIOToggle(GPIO_21);
				UartSendByte(UART_CONNECTOR, "Peligro, Vehiculo cerca.\r\n");
			}
		}
	}
}

static void medicion(void *pvParameter)
{

	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		valor_medicion = HcSr04ReadDistanceInCentimeters();
		Controlar_Leds();
	}
}

static void Alarma1(void *pvParameter)
{

	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		
		if (valor_medicion > 300 && valor_medicion < 500)
		GPIOOn(GPIO_20);
		else
		GPIOOff(GPIO_20);
}
}

static void Alarma2(void *pvParameter)
{
	
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		
		if (valor_medicion < 300)
		GPIOOn(GPIO_21);
		else
		GPIOOff(GPIO_21);
}
}

static void control_acelerometro(void *pvParameter){
				float valorX = 0;
				float valorY = 0;
				float valorZ = 0;
				float G = 0;
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		AnalogInputReadSingle(CH0, &valorX);
		AnalogInputReadSingle(CH1, &valorY);
		AnalogInputReadSingle(CH2, &valorZ);
		G+=(valorX-1650)/300;
		G+=(valorY-1650)/300;
		G+=(valorZ-1650)/300;

		if(G>4){
			UartSendByte(UART_CONNECTOR, "Caida detectada.\r\n");
		}
		
	}
				

}

}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	timer_config_t Timer_medicion = {
		.timer = TIMER_A,
		.period = CONFIG_BLINK_PERIOD_MUESTREO_DISTANCIA,
		.func_p = FuncTimerA,
		.param_p = NULL};
	TimerInit(&Timer_medicion);

	timer_config_t Timer_Alarma1 = {
		.timer = TIMER_A,
		.period = CONFIG_BLINK_PERIOD_MUESTREO_PRECAUCION,
		.func_p = FuncTimerB,
		.param_p = NULL};
	TimerInit(&Timer_Alarma1);

	timer_config_t Timer_acelerometro = {
		.timer = TIMER_A,
		.period = CONFIG_BLINK_PERIOD_MUESTREO_ACELEROMETRO,
		.func_p = FuncTimerC,
		.param_p = NULL};
	TimerInit(&Timer_acelerometro);

	HcSr04Init(GPIO_3, GPIO_2);

	serial_config_t conf_puerto = {
		.port = UART_CONNECTOR,
		.baud_rate = 9600,
		.func_p = &Controlar_Leds,
		.param_p = NULL,
	};
	UartInit(&conf_puerto);

	analog_input_config_t conversorAD = {
		.input = CH1,CH2,CH0,		/*!< Inputs: CH0, CH1, CH2, CH3 */
		.mode = ADC_SINGLE, /*!< Mode: single read or continuous read */
		.func_p = NULL,		/*!< Pointer to callback function for convertion end (only for continuous mode) */
		.param_p = NULL,	/*!< Pointer to callback function parameters (only for continuous mode) */
		.sample_frec = 0};
	AnalogInputInit(&conversorAD);

	gpioConf_t pines[5] =
		{{GPIO_20, GPIO_OUTPUT},
		 {GPIO_21, GPIO_OUTPUT},
		 };

	// Inicializo los puertos GPIO en el vector
	for (u_int8_t i = 0; i < 2; i++)
	{
		GPIOInit(pines[i].pin, pines[i].dir);
	}

	
	TimerStart(Timer_medicion.timer);
	TimerStart(Timer_Alarma1.timer);
	TimerStart(Timer_acelerometro.timer);

	xTaskCreate(&medicion,"medicion",512,NULL,5,&medir_distancia_task_handle);
	xTaskCreate(&Alarma1,"alarma precaucion",512,NULL,5,&precaucion_task_handle);
	xTaskCreate(&Alarma2,"alarma peligro",512,NULL,5,&peligro_task_handle);
	xTaskCreate(&control_acelerometro,"controla las variables del acelerometro",512,NULL,5,&acelerometro_task_handle);

}
/*==================[end of file]============================================*/