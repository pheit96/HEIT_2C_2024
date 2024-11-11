/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * se implementa un sistema de pesaje para camiones, en cual mide la velocidad de entrada del vehiculo a partir de la distncia medida por un sensor de ultrasonido, ademas una vez detenido el camion este es pesado a partir de 2 galgas colocadas en el suelo.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    galga1  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	medicion analogica	 	| 	CH0		|
 * | 	+5V  	 	| 	+5V	    	|
 * | 	GND 	 	| 	GND	    	|
 * 
 * |    galga2  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	medicion analogica	 	| 	CH1		|
 * | 	+5V  	 	| 	+5V	    	|
 * | 	GND 	 	| 	GND	    	|
 * 
 * ||    HC-SR04     |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	ECHO	 	| 	GPIO_3		|
 * | 	TRIGGER	 	| 	GPIO_2		|
 * | 	+5V  	 	| 	+5V	    	|
 * | 	GND 	 	| 	GND	    	|
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 11/11/2024 | Document creation		                         |
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
/** @def PERIODO_MEDICION_DISTANCIA
 *  @brief Periodo de delay para medir las distancias(en ms)
*/
#define PERIODO_MEDICION_DISTANCIA 90
/** @def CONFIG_BLINK_PERIOD_MUESTREO_DISTANCIA
 *  @brief Periodo de delay entre distancias para calcular la velocidad (en ms)
*/
#define TIEMPO_ENTRE_MEDIDAS 10

/** @def velocidad_maxima
 *  @brief Variable global de tipo flotante para registrar la velocidad maxima del vehiculo*/
float velocidad_maxima=0;
/** @def velocidad
 *  @brief Variable global de tipo flotante para registrar la velocidad instantanea*/
float velocidad=0;
/** @def distancia_metros
 *  @brief Variable global de tipo flotante para registrar la distancia a la que se encuentra el vehiculo*/
float distancia_metros=0;
/** @def distancia1
 *  @brief Variable global de tipo flotante para registrar la primera distancia para calcular la velocidad*/
float distancia1=0;
/** @def distancia2
 *  @brief Variable global de tipo flotante para registrar la segunda distancia para calcular la velocidad*/
float distancia2=0;
/** @def peso_total
 *  @brief Variable global de tipo flotante para registrar el peso total del camion*/
float peso_total=0;
/** @def teclas
 *  @brief Variable global de tipo entero de 8 bits destinada a la lectura de teclas por UART*/
uint8_t teclas=0;

/** @def CONFIG_BLINK_PERIOD_MUESTREO_PESO
 *  @brief Periodo del timer (en us)
*/
#define CONFIG_BLINK_PERIOD_MUESTREO_PESO 5000
/*==================[internal data definition]===============================*/
/** @def medir_distancia_task_handle
 *  @brief Objeto de tipo TaskHandle_t que se asocia con la tarea medicion*/
TaskHandle_t medir_distancia_task_handle = NULL;

/** @def pesar_camion_task_handle
 *  @brief Objeto de tipo TaskHandle_t que se asocia con la tarea Medir_Peso*/
TaskHandle_t pesar_camion_task_handle = NULL;

/*==================[internal functions declaration]=========================*/

/**
 * @fn FuncTimerA(void *param)
 * @param param parametro que no se utiliza
 * @brief envia una notificacion a la tarea Medir_Peso 
*/
void FuncTimerA(void *param)
{
	vTaskNotifyGiveFromISR(pesar_camion_task_handle, pdFALSE);
}

/** @fn Controlar_Leds(void)
 * @brief Funcion encargada de controlar el encendido de los leds segun la velocidad del camion al ingresar al sistema de pesaje
*/
void Controlar_Leds(void)
{

	if (velocidad > 8)
	{
		
		LedOff(LED_2);
		LedOff(LED_1);
		LedOn(LED_3);

	}
	else
	{
		if (velocidad > 0 && velocidad < 8)
		{
			LedOff(LED_3);
			LedOff(LED_1);
			LedOn(LED_2);
		}
		else
		{
			if (velocidad == 0)
			{
				LedOff(LED_3);
				LedOff(LED_2);
				LedOn(LED_1);
			}
		}
	}
}
/** @fn Medir_Velocidad()
 * @brief funcion encargada de calcular la velocidad instantanea del camion.
*/
void Medir_Velocidad(){
			
			velocidad=(distancia1-distancia2)/0.010;//0.010 ventana de tiempo entre las medidas de distancia tomadas
			if(velocidad>velocidad_maxima){
				velocidad_maxima=velocidad;
			}
			Controlar_Leds();
}

/** @fn medicion(void *pvParameter)
 * @brief Tarea encargada de medir la distancia del camion.
*/
static void medicion(void *pvParameter)
{

	while (true)
	{

		distancia_metros= HcSr04ReadDistanceInCentimeters()/100;
		if(distancia_metros<10){
			distancia1=HcSr04ReadDistanceInCentimeters()/100;
			vTaskDelay(TIEMPO_ENTRE_MEDIDAS / portTICK_PERIOD_MS);
			distancia2=HcSr04ReadDistanceInCentimeters()/100;
			Medir_Velocidad();
		}
		vTaskDelay(PERIODO_MEDICION_DISTANCIA / portTICK_PERIOD_MS);
	}
}

/** @fn Medir_Peso(void *pvParameter)
 * @brief Tarea realizada para calcular el peso del camion.
*/
static void Medir_Peso(void *pvParameter){
			float promedio1=0;
			float promedio2=0;
			float contenedor1=0;
			float contenedor2=0;
			uint16_t valor_galga1=0;
			uint16_t valor_galga2=0;

		while(1){
			ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
			uint8_t i=0;
			if(velocidad==0){
			contenedor1+=AnalogInputReadSingle(CH0, &valor_galga1)*20000/3300;
			contenedor2+=AnalogInputReadSingle(CH1, &valor_galga2)*20000/3300;
			i++;
			}
			if(i==49){
				promedio1=contenedor1/50;
				promedio2=contenedor2/50;
				peso_total=promedio1+promedio2;
				contenedor1=0;
				contenedor2=0;
				promedio1=0;
				promedio2=0;
				UartSendString(UART_PC,"Peso: ");
				UartSendString(UART_PC,(char *)UartItoa(peso_total,10));
				UartSendString(UART_PC," kg\r\n");
				UartSendString(UART_PC,"Velocidad Maxima: ");
				UartSendString(UART_PC,(char *)UartItoa(velocidad_maxima,10));
				UartSendString(UART_PC," m/s\r\n");

			}
		}
}

/**
 * @fn Leer_teclas() 
 * @brief Lee los valores ingresados por pantalla y enviados por puerto serie
 * 
*/
void Leer_teclas()
{
	UartReadByte(UART_PC, &teclas);
	if (teclas == 'o')
	{
		GPIOOn(GPIO_20);
		UartSendString(UART_PC,"Barrera Abierta.\r\n");

	}
	else
	{
		if (teclas == 'c')
		{
			GPIOOff(GPIO_20);
			UartSendString(UART_PC,"Barrera Cerrada.\r\n");

		}
	}
}

/*==================[external functions definition]==========================*/


void app_main(void){

/* Inicialización de timer Timer_medicion_peso */
	timer_config_t Timer_medicion_peso = {
		.timer = TIMER_A,
		.period = CONFIG_BLINK_PERIOD_MUESTREO_PESO,
		.func_p = FuncTimerA,
		.param_p = NULL};
	TimerInit(&Timer_medicion_peso);


/* Inicialización de Leds, Sensor de Ultrasonido, configuro entradas analogicas y configuro el puerto serie UART*/
	HcSr04Init(GPIO_3, GPIO_2);
	LedsInit();
	GPIOInit(GPIO_20, GPIO_OUTPUT);

	analog_input_config_t conversorAD = {
		.input = CH1,		/*!< Inputs: CH0, CH1, CH2, CH3 */
		.mode = ADC_SINGLE, /*!< Mode: single read or continuous read */
		.func_p = NULL,		/*!< Pointer to callback function for convertion end (only for continuous mode) */
		.param_p = NULL,	/*!< Pointer to callback function parameters (only for continuous mode) */
		.sample_frec = 0};

	AnalogInputInit(&conversorAD);analog_input_config_t conversorAD = {
		.input = CH0,		/*!< Inputs: CH0, CH1, CH2, CH3 */
		.mode = ADC_SINGLE, /*!< Mode: single read or continuous read */
		.func_p = NULL,		/*!< Pointer to callback function for convertion end (only for continuous mode) */
		.param_p = NULL,	/*!< Pointer to callback function parameters (only for continuous mode) */
		.sample_frec = 0};
	AnalogInputInit(&conversorAD);

	serial_config_t conf_puerto = {
		.port = UART_PC,
		.baud_rate = 9600,
		.func_p = NULL,
		.param_p = NULL,
	};
	UartInit(&conf_puerto);

	/* Creo las tareas y doy start al timer */
		xTaskCreate(&medicion,"Medicion",512,NULL,5,&medir_distancia_task_handle);
		xTaskCreate(&Medir_Peso,"Medir Peso",512,NULL,5,&pesar_camion_task_handle);
		TimerStart(Timer_medicion_peso.timer);

}
/*==================[end of file]============================================*/