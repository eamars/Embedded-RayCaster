/*
 * sfx.h
 *
 *  Created on: 13/08/2016
 *      Author: rba90
 */

#ifndef SFX_H_
#define SFX_H_

enum sfx_type
{
	SFX_FIRE = 0,
	SFX_WALL,
};

void SFXInit(unsigned long clock);
void SFXPlayerThread( void *args );


#endif /* SFX_H_ */
