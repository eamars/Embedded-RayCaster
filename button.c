/*
 * button.c
 *
 *  Created on: 4/08/2016
 *      Author: rba90
 */


#include <stdint.h>
#include <stdio.h>
#include <math.h>

/* FreeRTOS includes. */
#include "include/FreeRTOS.h"
#include "include/task.h"
#include "include/queue.h"
#include "include/semphr.h"

/* Stellaris library includes. */
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"

#include "main.h"
#include "button.h"
#include "world_map.h"
#include "sfx.h"


//*****************************************************************************
//
// The debounced state of the five push buttons.  The bit positions correspond
// to:
//
//     0 - Up
//     1 - Down
//     2 - Left
//     3 - Right
//     4 - Select
//
//*****************************************************************************
unsigned char g_ucSwitches = 0x1f;
uint8_t BUTTON_EVENT;

//*****************************************************************************
//
// The vertical counter used to debounce the push buttons.  The bit positions
// are the same as g_ucSwitches.
//
//*****************************************************************************
static unsigned char g_ucSwitchClockA = 0;
static unsigned char g_ucSwitchClockB = 0;

// player
extern Player_t currentPlayer;

// game settings
extern Settings_t gameSettings;

// game state
extern uint8_t gameState;

void ButtonPollingInit()
{
	// initialize ports
	// enable the PG3 to PG7 to read the five buttons
	GPIODirModeSet(GPIO_PORTG_BASE, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_DIR_MODE_IN);

	GPIOPadConfigSet(GPIO_PORTG_BASE, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_STRENGTH_2MA,
						 GPIO_PIN_TYPE_STD_WPU);
}

void ButtonPoll( void *args )
{
	// pass parameters
	ArgumentHandler *argumentHandler = (ArgumentHandler *) args;

	xQueueHandle buttonUpdateEventQueue = (xQueueHandle) argumentHandler->arg0;
	xSemaphoreHandle sfxEventQueue = (xSemaphoreHandle) argumentHandler->arg1;

	// initialize the task tick handler
	portTickType xLastWakeTime;

	uint8_t sfx;

	// initialize the task tick handler
	xLastWakeTime = xTaskGetTickCount();
	while (1)
	{
		unsigned long ulData, ulDelta;

		// read the state of the push buttons
		ulData = ((GPIOPinRead(GPIO_PORTG_BASE, (GPIO_PIN_3 | GPIO_PIN_4 |
												GPIO_PIN_5 | GPIO_PIN_6)) >> 3) |
				  (GPIOPinRead(GPIO_PORTG_BASE, GPIO_PIN_7) >> 3));

		// determine the switches that are at a different state than the debounced state
		ulDelta = ulData ^ g_ucSwitches;

		// increment the clocks by one
		g_ucSwitchClockA ^= g_ucSwitchClockB;
		g_ucSwitchClockB = ~g_ucSwitchClockB;

		// reset the clocks corresponding to switches that have not changed state
		g_ucSwitchClockA &= ulDelta;
		g_ucSwitchClockB &= ulDelta;

		// get the new debounced switch state
		g_ucSwitches &= g_ucSwitchClockA | g_ucSwitchClockB;
		g_ucSwitches |= (~(g_ucSwitchClockA | g_ucSwitchClockB)) & ulData;

		// determine the switches that just changed debounced state
		ulDelta ^= (g_ucSwitchClockA | g_ucSwitchClockB);

		// four direction button
		uint8_t button_type = BUTTON_IDLE;

		switch (g_ucSwitches & 0x0f)
		{
			case 0x0e:	// up
			{
				// at state 0, the player is not allowed to move back and forward
				if (gameState == GAME_WAIT_FOR_OTHER_PLAYER)
				{
					break;
				}

				button_type = BUTTON_UP;

				float moveX = currentPlayer.posX + currentPlayer.dirX * MOVE_SPEED;
				float moveY = currentPlayer.posY + currentPlayer.dirY * MOVE_SPEED;

				if(worldMap[(int)(moveX)][(int)(currentPlayer.posY)] == 0)
				{
					currentPlayer.posX = moveX;

					if (gameSettings.enableSFX)
					{
						sfx = SFX_WALL | SFX_NORMAL;
						xQueueSend(sfxEventQueue, &sfx, 0);
					}
				}

				if(worldMap[(int)(currentPlayer.posX)][(int)(moveY)] == 0)
				{
					currentPlayer.posY = moveY;

					if (gameSettings.enableSFX)
					{
						sfx = SFX_WALL | SFX_NORMAL;
						xQueueSend(sfxEventQueue, &sfx, 0);
					}
				}

				break;
			}
			case 0x0d: 	// down
			{
				// at state 0, the player is not allowed to move back and forward
				if (gameState == GAME_WAIT_FOR_OTHER_PLAYER)
				{
					break;
				}

				button_type = BUTTON_DOWN;

				float moveX = currentPlayer.posX - currentPlayer.dirX * MOVE_SPEED;
				float moveY = currentPlayer.posY - currentPlayer.dirY * MOVE_SPEED;

				if(worldMap[(int)(moveX)][(int)(currentPlayer.posY)] == 0)
				{
					currentPlayer.posX = moveX;

					if (gameSettings.enableSFX)
					{
						sfx = SFX_WALL | SFX_NORMAL;
						xQueueSend(sfxEventQueue, &sfx, 0);
					}
				}

				if(worldMap[(int)(currentPlayer.posX)][(int)(moveY)] == 0)
				{
					currentPlayer.posY = moveY;

					if (gameSettings.enableSFX)
					{
						sfx = SFX_WALL | SFX_NORMAL;
						xQueueSend(sfxEventQueue, &sfx, 0);
					}
				}

				break;
			}
			case 0x0b: 	// left
			{
				button_type = BUTTON_LEFT;

				float oldDirX = currentPlayer.dirX;
				currentPlayer.dirX = currentPlayer.dirX * COS_ROT_SPEED - currentPlayer.dirY * SIN_ROT_SPEED;
				currentPlayer.dirY = oldDirX * SIN_ROT_SPEED + currentPlayer.dirY * COS_ROT_SPEED;

				float oldPlaneX = currentPlayer.planeX;
				currentPlayer.planeX = currentPlayer.planeX * COS_ROT_SPEED - currentPlayer.planeY * SIN_ROT_SPEED;
				currentPlayer.planeY = oldPlaneX * SIN_ROT_SPEED + currentPlayer.planeY * COS_ROT_SPEED;
				break;
			}
			case 0x07: 	// right
			{
				button_type = BUTTON_RIGHT;

				float oldDirX = currentPlayer.dirX;
				currentPlayer.dirX = currentPlayer.dirX * COS_ROT_SPEED_N - currentPlayer.dirY * SIN_ROT_SPEED_N;
				currentPlayer.dirY = oldDirX * SIN_ROT_SPEED_N + currentPlayer.dirY * COS_ROT_SPEED_N;

				float oldPlaneX = currentPlayer.planeX;
				currentPlayer.planeX = currentPlayer.planeX * COS_ROT_SPEED_N - currentPlayer.planeY * SIN_ROT_SPEED_N;
				currentPlayer.planeY = oldPlaneX * SIN_ROT_SPEED_N + currentPlayer.planeY * COS_ROT_SPEED_N;

				break;
			}
			default:
				break;
		}

		// select
		if((ulDelta & 0x10) && !(g_ucSwitches & 0x10))
		{
			if (gameSettings.enableSFX)
			{
				// play sound
				sfx = SFX_FIRE | SFX_PREEMPT;
				xQueueSend(sfxEventQueue, &sfx, 0);

				// toggle fire state
				currentPlayer.state = 0x01;
			}

			button_type = BUTTON_SELECT;
		}

		// pass update event to main thread
		if (button_type != BUTTON_IDLE)
		{
			xQueueSend(buttonUpdateEventQueue, &button_type, 0);
		}

		vTaskDelayUntil(&xLastWakeTime, (20 / portTICK_RATE_MS));
	}
}
