/*
 * button.h
 *
 *  Created on: 4/08/2016
 *      Author: rba90
 */

#ifndef BUTTON_H_
#define BUTTON_H_

#define BUTTON_BACKOFF_DELAY 100 					// in ms

#include "portable/portmacro.h"

#include "include/queue.h"

enum button_type {
	BUTTON_UP = 0,
	BUTTON_DOWN,
	BUTTON_LEFT,
	BUTTON_RIGHT,
	BUTTON_SELECT,
};

typedef struct
{
	portTickType backoff_tick; // timer
	unsigned char ulPin; // current pin of button

} Button_t;

void ButtonInit(xQueueHandle _buttonEventQueue);
void ButtonInterruptHandler();
void ButtonHandlerThread( void *args );



#endif /* BUTTON_H_ */
