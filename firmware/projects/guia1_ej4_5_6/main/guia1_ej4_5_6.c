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
 * @author Heit Pedro (Pedren83@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <led.h>
#include <gpio_mcu.h>
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
/** @fn void  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
 * @brief Convierte un dato recibido a BCD y almacena cada digito en un vector (que recibe como puntero).
 * @param data Dato de 32 bits sin signo a convertir
 * @param digits Cantidad de digitos del dato en representacion decimal
 * @param bcd_number Puntero a un vector para almacenar cada uno de los digitos del dato convertido a BCD
 * @return
 */
void convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number)
{
	uint32_t aux = data;
	for (int i = digits - 1; i >= 0; i--)
	{
		bcd_number[i] = aux % 10;
		aux = aux / 10;
	}
}

/** @fn gpioConf_t
 * @brief Estructura que representa un puerto GPIO
 * @param pin numero de pin del GPIO
 * @param dir direccion del GPIO; '0' entrada ;  '1' salida
 */
typedef struct
{
	gpio_t pin; /*!< GPIO pin number */
	io_t dir;	/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

/** @fn void confPin (uint8_t bcd_number, gpioConf_t *array)
 * @brief Configura el estado del pin segun el bit correspondiente en el BCD recibido
 * @param bcd_number Representa un digito del dato BDC
 * @param array Puntero a un arreglo de estructuras GPIO donde
 * @return
 */
void confPin(uint8_t bcd_number, gpioConf_t *array)
{
	uint8_t mask = 1;

	for (uint8_t i = 0; i < 4; i++)
	{
		if (mask & bcd_number)
		{
			GPIOOn(array[i].pin);
		}
		else
		{
			GPIOOff(array[i].pin);
		}
		mask = mask << 1;
	}
}

/** @fn void Display(uint32_t data, uint8_t N_digitos, gpioConf_t *PinsArray, gpioConf_t *LCD_MUX_Array )
 * @brief Muestra por display el dato recibido
 * @param data Dato de 32 bits sin signo a mostrar por display
 * @param N_digitos Cantidad de digitos del dato
 * @param PinsArray Puntero a un arreglo de estructuras GPIO con los pines utilizados para cada digito del dato BCD
 * @param LCD_MUX_Array Puntero a un arreglo de estructuras GPIO con los pines utilizados en el multiplexor del display
 * @return
 */
void Display(uint32_t data, uint8_t N_digitos, gpioConf_t *PinsArray, gpioConf_t *LCD_MUX_Array)
{

	uint8_t array[N_digitos];
	convertToBcdArray(data, N_digitos, array);

	for (int i = 0; i < N_digitos; i++)
	{
		confPin(array[i], PinsArray);
		GPIOOn(LCD_MUX_Array[i].pin);
		GPIOOff(LCD_MUX_Array[i].pin);
	}
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	uint32_t num = 234;
	uint8_t array[3];
	uint8_t digito = 3;

	convertToBcdArray(num, digito, array);
	for (uint8_t i = 0; i < 3; i++)
	{
		printf("%d\n", array[i]);
	}

	// Creo el vector de estructuras del tipo gpioConf_t que contienen los puertos GPIO_20, GPIO_21, GPIO_22
	// y GPIO_23, con sus correspondientes direcciones(entrada/salida)
	gpioConf_t pines[4] =
		{{GPIO_20, GPIO_OUTPUT},
		 {GPIO_21, GPIO_OUTPUT},
		 {GPIO_22, GPIO_OUTPUT},
		 {GPIO_23, GPIO_OUTPUT}};

	// Inicializo los puertos GPIO en el vector
	for (u_int8_t i = 0; i < 4; i++)
	{
		GPIOInit(pines[i].pin, pines[i].dir);
		i++;
	}

	// Creo el vector de estructuras del tipo gpioConf_t que contienen los puertos GPIO_9, GPIO_18, GPIO_19
	// con sus correspondientes direcciones(entrada/salida)
	gpioConf_t LCD[3] =
		{{GPIO_19, GPIO_OUTPUT},
		 {GPIO_18, GPIO_OUTPUT},
		 {GPIO_9, GPIO_OUTPUT}};

	// Inicializo los puertos GPIO en el vector
	for (int i = 0; i < 3; i++)
	{
		GPIOInit(LCD[i].pin, LCD[i].dir);
	}

	Display(num, digito, pines, LCD);

	while (1)
	{
		/* code */
	}
}
/*==================[end of file]============================================*/