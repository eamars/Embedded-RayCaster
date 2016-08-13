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
	BUTTON_POS_MOVE = 0,
	BUTTON_DIR_MOVE,
	BUTTON_IDLE,
	BUTTON_SELECT
};

extern uint8_t BUTTON_EVENT;

void ButtonPollingInit();
void ButtonPoll( void *args );


#endif /* BUTTON_H_ */
