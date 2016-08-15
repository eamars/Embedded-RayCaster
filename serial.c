/*
 * serial.c
 *
 *  Created on: 15/08/2016
 *      Author: rba90
 */
#include <stdint.h>
#include <stdio.h>

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
#include "driverlib/interrupt.h"
#include "inc/hw_ints.h"

// modules
#include "main.h"
#include "world_map.h"
#include "serial.h"
#include "screen.h"
#include "button.h"

#define GPIO_PA0_U0RX 			0x00000001
#define GPIO_PA1_U0TX           0x00000401

// player
extern Player_t currentPlayer;
extern Player_t otherPlayer;
extern uint8_t gameState;


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
	// pass parameters
	ArgumentHandler *argumentHandler = (ArgumentHandler *) args;

	xQueueHandle buttonUpdateEventQueue = (xQueueHandle) argumentHandler->arg0;
	xSemaphoreHandle sfxEventQueue = (xSemaphoreHandle) argumentHandler->arg1;

	// initialize the task tick handler
	portTickType xLastWakeTime;

	FloatString_t float_string;
	uint8_t buffer[17];
	uint8_t byte;

	// initialize the task tick handler
	xLastWakeTime = xTaskGetTickCount();

	while (1)
	{
		// copy state (can be used as a sync flag)
		buffer[0] = 0xfe | currentPlayer.state;

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

		for (uint8_t i = 0; i < 17; i++)
		{
			UARTCharPut(UART0_BASE, buffer[i]);
		}

		// toggle fire state
		currentPlayer.state = 0x00;

		// read from serial
		while(UARTCharsAvail(UART0_BASE))
		{
			// read from
			byte = (uint8_t)UARTCharGet(UART0_BASE);

			switch (gameState)
			{
				case GAME_WAIT_FOR_OTHER_PLAYER:
				{
					if (byte == 0xfe || byte == 0xff)
					{
						// advance the state
						gameState = GAME_FREE_ROAM;
						ScreenClearFrameBuffer(2);

						// pass a BUTTON_IDLE to update the screen
						uint8_t button_type = BUTTON_IDLE;
						xQueueSend(buttonUpdateEventQueue, &button_type, 0);
					}
					break;
				}
				case GAME_FREE_ROAM:
				{
					// buffer index
					static uint8_t idx = 0;

					if (byte == 0xfe || byte == 0xff) // sync flag
					{
						// reset buffer index
						idx = 0;

						// store first byte
						buffer[idx++] = byte;
					}
					else
					{
						buffer[idx++] = byte;
					}

					// has already fill the buffer, then reset the idx and start loading
					if (idx == 16)
					{
						idx = 0;

						// get fire state
						otherPlayer.state = buffer[0] & 0x01;

						// get posx
						memcpy(float_string.bytes, buffer + 1, 4);
						otherPlayer.posX = float_string.f;

						// get posy
						memcpy(float_string.bytes, buffer + 5, 4);
						otherPlayer.posY = float_string.f;

						// get dirx
						memcpy(float_string.bytes, buffer + 9, 4);
						otherPlayer.dirX = float_string.f;

						// get dirY
						memcpy(float_string.bytes, buffer + 13, 4);
						otherPlayer.dirY = float_string.f;

						// pass a BUTTON_IDLE to update the screen
						uint8_t button_type = BUTTON_IDLE;
						xQueueSend(buttonUpdateEventQueue, &button_type, 0);

					}
					break;
				}
				default:
					break;
			}
		}

		vTaskDelayUntil(&xLastWakeTime, (100 / portTICK_RATE_MS));
	}
}
