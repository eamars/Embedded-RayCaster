/*
 * button.h
 *
 *  Created on: 4/08/2016
 *      Author: rba90
 */

#ifndef BUTTON_H_
#define BUTTON_H_

#include <stdint.h>

#include "portable/portmacro.h"

#include "include/queue.h"

enum button_type {
	BUTTON_UP = 0,
	BUTTON_DOWN,
	BUTTON_LEFT,
	BUTTON_RIGHT,
	BUTTON_SELECT,
	BUTTON_IDLE,
};

void ButtonPollingInit();
void ButtonPoll( void *args );


#endif /* BUTTON_H_ */
