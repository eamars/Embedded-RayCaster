/*
 * raycaster.h
 *
 *  Created on: 9/08/2016
 *      Author: rba90
 *
 *  Purpose:
 *  	Render the 3D scene out of 2D maps. It will also display the opponent as
 *  	sprite.
 *
 *  Scheduled task function:
 *  	void RayCaster(void *args);
 *
 *  Dependencies:
 *  	ButtonPolling(), SerialHandlerThread(), ScreenUpdateThread()
 *
 *  Shared Variables:
 *  	Player_t currentPlayer;
 *		Player_t otherPlayer;
 *		Settings_t gameSettings;
 * 		uint8_t gameState = GAME_WAIT_FOR_OTHER_PLAYER;
 */

#ifndef RAYCASTER_H_
#define RAYCASTER_H_

#include "main.h"

#define VIEW_DIST_WALL 3.0f
#define VIEW_DIST_FLOOR 4.0f
#define VIEW_DIST_SPRITE 5.0f

#define RAD_TO_DEG_RATIO 	57.2957795131f

float currentDistTable[screenHeight / 2];

// Update framebuffer based on user's location and direction.
// args:
// 		.arg0 = buttonUpdateEventQueue, .arg1 = screenUpdateEvent
void RayCaster(void *args);



#endif /* RAYCASTER_H_ */
