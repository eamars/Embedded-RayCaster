/*
 * button.c
 *
 *  Created on: 4/08/2016
 *      Author: rba90
 */


#include <stdint.h>
#include <stdio.h>

/* FreeRTOS includes. */
#include "include/FreeRTOS.h"
#include "include/task.h"
#include "include/queue.h"

/* Stellaris library includes. */
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"
#include "button.h"

Button_t buttonList[5];
xQueueHandle buttonEventQueue;

void ButtonInit(xQueueHandle _buttonEventQueue)
{
	// assign button queue
	buttonEventQueue = _buttonEventQueue;

	// regsiter the handler for port G into the vector table
	GPIOPortIntRegister (GPIO_PORTG_BASE, ButtonInterruptHandler);

	// enable the PG3 to PG7 to read the five buttons
	GPIODirModeSet(GPIO_PORTG_BASE, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_DIR_MODE_IN);

	GPIOPadConfigSet(GPIO_PORTG_BASE, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_STRENGTH_2MA,
						 GPIO_PIN_TYPE_STD_WPU);

	// trigger the interrupt on falling edge
	GPIOIntTypeSet (GPIO_PORTG_BASE, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_FALLING_EDGE);

	// enable the pin change interrupt
	GPIOPinIntEnable (GPIO_PORTG_BASE, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);

	// enable interrupt on port G
	IntEnable (INT_GPIOG);

	// initilize a button list
	buttonList[BUTTON_UP] = (Button_t){.backoff_tick = 0, .ulPin = GPIO_PIN_3};
	buttonList[BUTTON_DOWN] = (Button_t){.backoff_tick = 0, .ulPin = GPIO_PIN_4};
	buttonList[BUTTON_LEFT] = (Button_t){.backoff_tick = 0, .ulPin = GPIO_PIN_5};
	buttonList[BUTTON_RIGHT] = (Button_t){.backoff_tick = 0, .ulPin = GPIO_PIN_6};
	buttonList[BUTTON_SELECT] = (Button_t){.backoff_tick = 0, .ulPin = GPIO_PIN_7};
}


void ButtonInterruptHandler()
{
	int8_t i;
	portTickType currentTick;

	currentTick = xTaskGetTickCountFromISR();

	// clean the current interrupt state
	GPIOPinIntClear (GPIO_PORTG_BASE,
			buttonList[BUTTON_UP].ulPin |
			buttonList[BUTTON_DOWN].ulPin |
			buttonList[BUTTON_LEFT].ulPin |
			buttonList[BUTTON_RIGHT].ulPin |
			buttonList[BUTTON_SELECT].ulPin
	);

	for (i = BUTTON_UP; i <= BUTTON_SELECT; i++)
	{
		if (GPIOPinRead(GPIO_PORTG_BASE, buttonList[i].ulPin) == 0)
		{
			// take period between two individual clicks
			portTickType period = (currentTick - buttonList[i].backoff_tick) / portTICK_RATE_MS;

			// if the period between two falling edges is longer than the backoff delay, the
			// button event is considered as a single click
			if (period > BUTTON_BACKOFF_DELAY)
			{
				buttonList[i].backoff_tick = currentTick; // start the backoff timer

				// send button event to queue
				xQueueSendFromISR(
						buttonEventQueue,		// queue
						&i,						// button
						pdFALSE					// do not trigger context switch within interrupt handler
				);
				break;
			}

			// otherwise just skip
			else
			{

			}

			break;
		}
	}
}

