/*
 * serial.c
 *
 *  Created on: 15/08/2016
 *      Author: rba90
 */
#include <stdint.h>

/* FreeRTOS includes. */
#include "include/FreeRTOS.h"
#include "include/task.h"
#include "include/queue.h"
#include "include/semphr.h"

/* Stellaris library includes. */
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"

// modules
#include "main.h"
#include "world_map.h"
#include "serial.h"

#define GPIO_PA0_U0RX 			0x00000001
#define GPIO_PA1_U0TX           0x00000401

// player
extern Player_t currentPlayer;

// convert float to string
typedef union
{
	float f;
	uint8_t bytes[sizeof(float)];			// on stellaris, sizeof(float) is 4 bytes
} FloatString_t;


void SerialInit()
{
	GPIOPinTypeUART (GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	UARTConfigSetExpClk (UART0_BASE, SysCtlClockGet(), 9600ul,

						(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |

						 UART_CONFIG_PAR_NONE));

	UARTFIFOEnable (UART0_BASE);

	UARTEnable (UART0_BASE);
}

void SerialHandlerThread(void *args)
{
	// initialize the task tick handler
	portTickType xLastWakeTime;

	int i;
	FloatString_t float_string;
	uint8_t buffer[17];

	// initialize the task tick handler
	xLastWakeTime = xTaskGetTickCount();

	while (1)
	{
		// copy state (can be used as a sync flag)
		buffer[0] = 0xfe;

		// copy posx
		float_string.f = currentPlayer.posX;
		memcpy(buffer + 1, float_string.bytes, 4);

		// copy posy
		float_string.f = currentPlayer.posY;
		memcpy(buffer + 5, float_string.bytes, 4);

		// copy dirX
		float_string.f = currentPlayer.dirX;
		memcpy(buffer + 9, float_string.bytes, 4);

		// copy dirY
		float_string.f = currentPlayer.dirY;
		memcpy(buffer + 13, float_string.bytes, 4);

		for (i = 0; i < 17; i++)
		{
			UARTCharPut(UART0_BASE, buffer[i]);
		}



		vTaskDelayUntil(&xLastWakeTime, (100 / portTICK_RATE_MS));
	}
}
