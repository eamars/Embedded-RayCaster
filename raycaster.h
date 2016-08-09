/*
 * raycaster.h
 *
 *  Created on: 9/08/2016
 *      Author: rba90
 */

#ifndef RAYCASTER_H_
#define RAYCASTER_H_

#include "main.h"

#define VIEW_DIST_WALL 3.0f
#define VIEW_DIST_FLOOR 4.0f

float currentDistTable[screenHeight / 2];


void RayCaster(void *args);



#endif /* RAYCASTER_H_ */
