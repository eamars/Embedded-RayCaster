/*
 * serial.h
 *
 *  Created on: 15/08/2016
 *      Author: rba90
 *
 *  Purpose:
 *  	Send and receive update from/to opponent
 *
 *  Scheduled task function:
 *  	void SerialHandlerThread(void *args);
 *
 *  Dependencies:
 *  	RayCaster()
 *
 *  Shared Variables:
 *  	Player_t currentPlayer;
 *		Player_t otherPlayer;
 * 		uint8_t gameState = GAME_WAIT_FOR_OTHER_PLAYER;
 */

#ifndef SERIAL_H_
#define SERIAL_H_

// initialize serial interface on PA1 and PA2
void SerialInit();

// serial handler thread that send an receive bytes over
// serial with other player
// args:
//		.arg0 = buttonUpdateEventQueue, .arg1 = sfxEventQueue
void SerialHandlerThread( void *args );



#endif /* SERIAL_H_ */
