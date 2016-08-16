/*
 * serial.c
 *
 *  Created on: 15/08/2016
 *      Author: rba90
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

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
#include "raycaster.h"

#define GPIO_PA0_U0RX	0x00000001
#define GPIO_PA1_U0TX	0x00000401

#define BUFFER_SZ 13

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

	UARTConfigSetExpClk (UART0_BASE, SysCtlClockGet(), 115200ul,

						(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |

						 UART_CONFIG_PAR_NONE));

	UARTFIFOEnable (UART0_BASE);

	UARTEnable (UART0_BASE);
}


void SerialHandlerThread(void *args)
{
	// pass parameters
	xQueueHandle buttonUpdateEventQueue = (xQueueHandle) args;

	// initialize the task tick handler
	portTickType xLastWakeTime;

	// union that convert float to byte array
	FloatString_t float_string;

	// transmit buffer
	uint8_t txbuffer[BUFFER_SZ];

	// receive buffer
	uint8_t rxbuffer[BUFFER_SZ];

	// received byte
	uint8_t byte;

	// buffer index
	uint8_t idx = 0;

	// reset buffers
	memset(txbuffer, 0x0, sizeof(txbuffer));
	memset(rxbuffer, 0x0, sizeof(rxbuffer));

	// initialize the task tick handler
	xLastWakeTime = xTaskGetTickCount();

	while (1)
	{
		// copy state (can be used as a sync flag)
		txbuffer[0] = 0xfe | currentPlayer.state;

		// copy posx
		float_string.f = currentPlayer.posX;
		memcpy(txbuffer + 1, float_string.bytes, 4);

		// copy posy
		float_string.f = currentPlayer.posY;
		memcpy(txbuffer + 5, float_string.bytes, 4);

		// since direction can be represented by integer between
		// -90 to 90, it can be packed and transmitted as singed integer.

		// copy dirX
		txbuffer[9] = (int8_t) roundf(RAD_TO_DEG_RATIO * currentPlayer.dirX);

		// copy dirY
		txbuffer[10] = (int8_t) roundf(RAD_TO_DEG_RATIO * currentPlayer.dirY);

		// copy game state
		txbuffer[11] = gameState;

		for (uint8_t i = 0; i < BUFFER_SZ; i++)
		{
			UARTCharPut(UART0_BASE, txbuffer[i]);
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
					if (byte == 0xfe || byte == 0xff) // sync flag
					{
						// reset buffer index
						idx = 0;
					}

					// store bytes
					rxbuffer[idx++] = byte;

					// has already fill the buffer, then reset the idx and start loading
					if (idx == BUFFER_SZ - 1)
					{
						idx = 0;

						// get fire state
						otherPlayer.state = rxbuffer[0] & 0x01;

						// get posx
						memcpy(float_string.bytes, rxbuffer + 1, 4);
						otherPlayer.posX = float_string.f;

						// get posy
						memcpy(float_string.bytes, rxbuffer + 5, 4);
						otherPlayer.posY = float_string.f;

						// get dirx
						otherPlayer.dirX = ((int8_t) rxbuffer[9]) / RAD_TO_DEG_RATIO;

						// get dirY
						otherPlayer.dirY = ((int8_t) rxbuffer[10]) / RAD_TO_DEG_RATIO;

						// get gameState
						uint8_t state = rxbuffer[11];
						if (state == GAME_VICTORY) // other player wins the game
						{
							gameState = GAME_DEFEAT;
							ScreenPrintStr(2, "You Lose!", 9, 27, 44, FONT_6x8, 15);

							// TODO: Find a better way
							ScreenUpdate();
						}

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
