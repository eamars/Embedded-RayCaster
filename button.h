/*
 * button.h
 *
 *  Created on: 4/08/2016
 *      Author: rba90
 *
 *  Purpose:
 *  	Poll the PIO to read logic level of five push buttons. Push the button event to the
 *  	queue to trigger the rendering event in the RayCaster() thread.
 *
 *  	If four direction keys are pressed, it will push the stepping sound to the queue, and
 *  	pass it to SFXPlayerThread().
 *
 *  	If BUTTON_SELECT key is pressed, it will push the index of firing sound effect to
 *  	SFXPlayerThread()
 *
 *  Scheduled task function:
 *  	void ButtonPoll( void *args );
 *
 *  Dependencies:
 *  	RayCaster(), SFXPlayerThread()
 *
 *  Shared variables:
 *  	Player_t currentPlayer;
 *		Settings_t gameSettings;
 * 		uint8_t gameState = GAME_WAIT_FOR_OTHER_PLAYER;
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

// Initialize button hardware
void ButtonPollingInit();

// periodic taks that poll all five push buttons
// args:
// 		.arg0 = buttonUpdateEventQueue, .arg1 = sfxEventQueue
void ButtonPoll( void *args );


#endif /* BUTTON_H_ */
