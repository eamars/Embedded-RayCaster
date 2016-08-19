/*
 * sfx.h
 *
 *  Created on: 13/08/2016
 *      Author: rba90
 *
 *  Purpose:
 *  	Play tune based on Class-D driver
 *
 *  Scheduled task function:
 *  	void SFXPlayerThread(void *args);
 *
 *  Dependencies:
 *  	ButtonPolling(), SerialHandlerThread()
 *
 *  Shared Variables:
 *  	None
 */

#ifndef SFX_H_
#define SFX_H_

enum sfx_type
{
	SFX_FIRE = 0x00,			// 0bxxxx0000
	SFX_WALL = 0x01,			// 0bxxxx0001
};

enum sfx_flags
{
	SFX_PREEMPT = 0x80,		// 0b1000xxxx
	SFX_NORMAL = 0x40,		// 0b0100xxxx
};

// initialize Class-D driver
void SFXInit(unsigned long clock);

// SFX handler thread that play tune on request
// args:
// 		sfxEventQueue
void SFXPlayerThread( void *args );


#endif /* SFX_H_ */
