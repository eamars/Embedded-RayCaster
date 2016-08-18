/*
 * main.h
 *
 *  Created on: 9/08/2016
 *      Author: rba90
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <stdbool.h>
#include <stdint.h>

#define screenWidth 128
#define screenHeight 96

#define verticalOffset 0

#define MOVE_SPEED 0.05f

// speed = 0.05f
#define COS_ROT_SPEED 0.999f
#define SIN_ROT_SPEED 0.050f
#define COS_ROT_SPEED_N 0.999f
#define SIN_ROT_SPEED_N -0.050f

typedef struct
{
	void *arg0;
	void *arg1;
	void *arg2;
	void *arg3;
} ArgumentHandler;

typedef struct
{
	float posX, posY;			// current player location
	float dirX, dirY;			// current player viewing direction
	float planeX, planeY;		// player field of view
	uint8_t state;				// state of player (shoot or not)
} Player_t;

typedef struct
{
	bool renderFog;
	bool displayDebugText;
	bool enableSFX;
} Settings_t;

typedef enum
{
	GAME_WAIT_FOR_OTHER_PLAYER = 0,
	GAME_FREE_ROAM,
	GAME_DEFEAT,
	GAME_VICTORY,
} Game_t;


#endif /* MAIN_H_ */
