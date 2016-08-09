/*
    FreeRTOS V6.0.5 - Copyright (C) 2009 Real Time Engineers Ltd.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    ***NOTE*** The exception to the GPL is included to allow you to distribute
    a combined work that includes FreeRTOS without being obliged to provide the
    source code for proprietary components outside of the FreeRTOS kernel.
    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public 
    License and the FreeRTOS license exception along with FreeRTOS; if not it 
    can be viewed here: http://www.freertos.org/a00114.html and also obtained 
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.

    1 tab == 4 spaces!

    http://www.FreeRTOS.org - Documentation, latest information, license and
    contact details.

    http://www.SafeRTOS.com - A version that is certified for use in safety
    critical systems.

    http://www.OpenRTOS.com - Commercial support, development, porting,
    licensing and training services.
*/


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* FreeRTOS includes. */
#include "include/FreeRTOS.h"
#include "include/task.h"
#include "include/queue.h"
#include "include/semphr.h"
#include "include/timers.h"

/* Stellaris library includes. */
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/adc.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"

/* Demo includes. */
#include "demo_code/basic_io.h"

// include OLED driver
#include "drivers/rit128x96x4.h"
#include "screen.h"
#include "button.h"
#include "raycaster.h"
#include "main.h"

#include "raycaster.h"
#include "texture.h"
#include "world_map.h"


/* Used as a loop counter to create a very crude delay. */
#define mainDELAY_LOOP_COUNT		( 0xfffff )

// game related
typedef enum
{
	INTRO = 0,
	ROAM,
	INTERACT,
	GAME_END_SUCCESS,
	GAME_END_FAILED
} Game_t;


/* The task function. */
void ScreenUpdateThread( void *args );
void TimerUpdateThread(void *args);

uint8_t gameState;


void PinReset()
{
	SysCtlPeripheralReset (SYSCTL_PERIPH_ADC0);
	SysCtlPeripheralReset (SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralReset (SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralReset (SYSCTL_PERIPH_GPIOG);
}

void PinInit()
{
	// enable system peripherals
	SysCtlPeripheralEnable (SYSCTL_PERIPH_ADC0);
	SysCtlPeripheralEnable (SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable (SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable (SYSCTL_PERIPH_GPIOG);
}


int main( void ){
	/* Set the clocking to run from the PLL at 50 MHz.  Assumes 8MHz XTAL,
	whereas some older eval boards used 6MHz. */
	SysCtlClockSet( SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ );

	// create queue for button event passing
	xQueueHandle buttonEventQueue = xQueueCreate(5, sizeof(uint8_t));

	// create semaphore for screen update event
	xSemaphoreHandle screenUpdateEvent;
	vSemaphoreCreateBinary(screenUpdateEvent);

	// create an argument handler that pass multiple argument to tasks
	ArgumentHandler handler = {.arg0 = buttonEventQueue, .arg1 = screenUpdateEvent};

	// Initialize peripherials
	PinReset();
	PinInit();

	ScreenInit();
	ButtonInit(buttonEventQueue);


	xTaskCreate(RayCaster, "RayCaster", 960, (void *) &handler, 1, NULL);
	xTaskCreate(ScreenUpdateThread, "ScreenUpdateThread", 48, (void *) screenUpdateEvent, 2, NULL);
	// xTaskCreate(TimerUpdateThread, "TimerUpdateThread", 240, (void *) screenUpdateEvent, 3, NULL);

	/* Create the other task in exactly the same way.  Note this time that we
	are creating the SAME task, but passing in a different parameter.  We are
	creating two instances of a single task implementation. */

	/* Start the scheduler so our tasks start executing. */
	vTaskStartScheduler();	
	
	/* If all is well we will never reach here as the scheduler will now be
	running.  If we do reach here then it is likely that there was insufficient
	heap available for the idle task to be created. */
	for( ;; );
}
/*-----------------------------------------------------------*/


void exitIntroState(xTimerHandle pxTimer)
{
	gameState = 1;
	ScreenClearFrameBuffer(2);
}

void exitInteractState(xTimerHandle pxTimer)
{
	gameState = 1;
	ScreenClearFrameBuffer(2);
}


/*
 * The game is inspired from the idea of ray caster, which was used in the world's first
 * FPS game Wolfstein 3D.
 * http://lodev.org/cgtutor/raycasting2.html
 */
void RayCaster(void *args)
{
	// pass argument
	ArgumentHandler *argumentHandler = (ArgumentHandler *) args;

	xQueueHandle buttonEventQueue = (xQueueHandle) argumentHandler->arg0;
	xSemaphoreHandle screenUpdateEvent = (xSemaphoreHandle) argumentHandler->arg1;

	// initialize the task tick handler
	portTickType xLastWakeTime;

	// string
	char textBuffer[32];

	// button
	int8_t button;
	bool enableFog = true;

	// initialize game state
	gameState = 0;

	// variables about ray casting
	float posX = 22.0f;
	float posY = 11.5f; 		// starting position
	float dirX = -1.0f;
	float dirY = 0.0f;		// initial direction vector
	float planeX = 0.0f;
	float planeY = 0.66f;	// 2d raycaster version of camera plane

	// measure framerate
	portTickType time = 0;
	portTickType oldTime = 0;

	// iterative variable
	int x, y;


	// generate texture
	memset(texture, 0x0, sizeof(texture));

	for (x = 0; x < texWidth; x++)
	{
		for (y = 0; y < texHeight; y++)
		{
			texture[TEXTURE_GRID][texWidth * y + x] = 10 * (x != y && x != texWidth - y); 		// horizontal and vertical grid
			texture[TEXTURE_CROSS][texWidth * y + x] = 10 * (x % 4 && y % 4);					// black cross
		}
	}

	// copy custom texture
	memcpy(texture[TEXTURE_BRICK], brick, texWidth * texHeight);
	memcpy(texture[TEXTURE_ENCE463], ence463_block, texWidth * texHeight);
	memcpy(texture[TEXTURE_STONE_BRICK], stone_brick, texWidth * texHeight);
	memcpy(texture[TEXTURE_STONE_BRICK_CARVED], stone_brick_carved, texWidth * texHeight);
	memcpy(texture[TEXTURE_NETHER_BRICK], nether_brick, texWidth * texHeight);
	memcpy(texture[TEXTURE_GREYSTONE], greystone, texWidth * texHeight);
	memcpy(texture[TEXTURE_STEVE], steve_block, texWidth * texHeight);

	// make small lookup table for currentDist when rendering floor
	for (y = screenHeight/2; y < screenHeight; y++)
	{
		currentDistTable[y - screenHeight / 2] = screenHeight / (2.0f * y - screenHeight);
	}

	// display a startup message on screen buffer 2
	ScreenPrintStr(2, "You find yourself", 17, 14, 20, FONT_6x8, 15);
	ScreenPrintStr(2, "awake in a strange", 18, 11, 28, FONT_6x8, 15);
	ScreenPrintStr(2, "room. Mist", 10, 34, 36, FONT_6x8, 15);
	ScreenPrintStr(2, "surrounding you are", 20, 8, 44, FONT_6x8, 15);
	ScreenPrintStr(2, "getting thick.", 14, 25, 52, FONT_6x8, 15);
	ScreenPrintStr(2, "You need to find a", 19, 10, 60, FONT_6x8, 15);
	ScreenPrintStr(2, "way out!", 8, 40, 68, FONT_6x8, 15);

	// create a timer object which will be expired after 5 second
	xTimerHandle GameIntroTimer = xTimerCreate("GameIntroTimer", 5000 * portTICK_RATE_MS, pdFALSE, NULL, exitIntroState);

	// start the timer
	xTimerStart(GameIntroTimer, 0);

	// initialize the task tick handler
	xLastWakeTime = xTaskGetTickCount();

	while (1)
	{
		// clear the old frame before drawing
		ScreenClearFrameBuffer(1);

		for (x = 0; x < screenWidth; x++)
		{
			// calculate ray position and direction
			float cameraX = 2 * x / (float) screenWidth - 1; 		// x coordinate in camera space
			float rayPosX = posX;
			float rayPosY = posY;
			float rayDirX = dirX + planeX * cameraX;
			float rayDirY = dirY + planeY * cameraX;

			// which box of the map we're in
			int mapX = (int) rayPosX;
			int mapY = (int) rayPosY;

			// length of ray from current position to next x or y-side
			float sideDistX;
			float sideDistY;

			// length of ray from one x or y-side to next x or y-side
			float deltaDistX = sqrtf(1 + (rayDirY * rayDirY) / (rayDirX * rayDirX));
			float deltaDistY = sqrtf(1 + (rayDirX * rayDirX) / (rayDirY * rayDirY));
			float perpWallDist;

			// what direction to step in x or y-direction, either + 1 or -1
			int stepX;
			int stepY;

			int hit = 0;		// was there a wall hit?
			int side;			// was a NS or a EW wall hit?

			// calculate step and initial sideDist
			if (rayDirX < 0)
			{
				stepX = -1;
				sideDistX = (rayPosX - mapX) * deltaDistX;
			}
			else
			{
				stepX = 1;
				sideDistX = (mapX + 1.0f - rayPosX) * deltaDistX;
			}

			if (rayDirY < 0)
			{
				stepY = -1;
				sideDistY = (rayPosY - mapY) * deltaDistY;
			}
			else
			{
				stepY = 1;
				sideDistY = (mapY + 1.0f - rayPosY) * deltaDistY;
			}

			// perform DDA
			while (hit == 0)
			{
				// jump to next map square, OR in x-direction, Or in y-direction
				if (sideDistX < sideDistY)
				{
					sideDistX += deltaDistX;
					mapX += stepX;
					side = 0;
				}
				else
				{
					sideDistY += deltaDistY;
					mapY += stepY;
					side = 1;
				}

				// check if ray has hit a wall
				if (worldMap[mapX][mapY] > 0) hit = 1;
			}

			// calculate distance projected on camera direction (oblique distance will give fisheye effect!)
			if (side == 0) perpWallDist = (mapX - rayPosX + (1 - stepX) / 2) / rayDirX;
			else		   perpWallDist = (mapY - rayPosY + (1 - stepY) / 2) / rayDirY;

			// calculate height of line to draw on screen
			int lineHeight = (int) (screenHeight / perpWallDist);

			// calculate lowest and heighest pixels to fill in current stripe
			int drawStart = -lineHeight / 2 + screenHeight / 2;
			if (drawStart < 0) drawStart = 0;

			int drawEnd = lineHeight / 2 + screenHeight / 2;
			if (drawEnd >= screenHeight) drawEnd = screenHeight - 1;

			// texturing calculations
			int texNum = worldMap[mapX][mapY] - 1; // match the value in the map (0 in map is empty)

			// calculate value of wallX
			float wallX;		// where exactly the wall was hit
			if (side == 0) wallX = rayPosY + perpWallDist * rayDirY;
			else		   wallX = rayPosX + perpWallDist * rayDirX;
			wallX -= floorf(wallX);

			// x coordinate on the texture
			int texX = (int)(wallX * (float) texWidth);
			if (side == 0 && rayDirX > 0) texX = texWidth - texX - 1;
			if (side == 1 && rayDirY < 0) texX = texWidth - texX - 1;

			for (y = drawStart; y < drawEnd; y++)
			{
				int d = y * 256 - screenHeight * 128 + lineHeight * 128;
				int texY = ((d * texHeight) / lineHeight) / 256;

				uint8_t color = texture[texNum][texHeight * texY + texX];

				// make color darker for y-side
				if (side == 1)
				{
					color >>= 1;
				}

				if (perpWallDist > VIEW_DIST_WALL && enableFog)
				{
					color >>= (int)(perpWallDist - VIEW_DIST_WALL);
				}

				// set pixel
				ScreenSetPixel(1, x, y + verticalOffset, color);
			}

			// floor casting
			float floorXWall, floorYWall;		//x, y position of the floor texel at the bottom of the wall

			// 4 different wall directions possible
			if (side == 0 && rayDirX > 0)
			{
				floorXWall = mapX;
				floorYWall = mapY + wallX;
			}
			else if (side == 0 && rayDirX < 0)
			{
				floorXWall = mapX + 1.0f;
				floorYWall = mapY + wallX;
			}
			else if (side == 1 && rayDirY > 0)
			{
				floorXWall = mapX + wallX;
				floorYWall = mapY;
			}
			else if (side == 1 && rayDirY < 0)
			{
				floorXWall = mapX + wallX;
				floorYWall = mapY + 1.0f;
			}

			float distWall, currentDist;

			distWall = perpWallDist;

			// become > 0 when the integer overflows
			if (drawEnd < 0) drawEnd = screenHeight - 1;

			// draw the floor from drawEnd to the bottom of the screen
			for (y = drawEnd; y < screenHeight; y++)
			{
				// currentDist = screenHeight / (2.0f * y - screenHeight);	// small lookup table can be used instead
				currentDist = currentDistTable[y - screenHeight / 2];

				float weight = currentDist / distWall;

				float currentFloorX = weight * floorXWall + (1.0f - weight) * posX;
				float currentFloorY = weight * floorYWall + (1.0f - weight) * posY;

				int floorTexX, floorTexY;
				floorTexX = ((int) (currentFloorX * texWidth)) % texWidth;
				floorTexY = ((int) (currentFloorY * texHeight)) % texHeight;

				uint8_t floor_color = texture[TEXTURE_STONE_BRICK][texWidth * floorTexY + floorTexX] >> 1;
				uint8_t celing_color = texture[TEXTURE_GREYSTONE][texWidth * floorTexY + floorTexX];

				if (currentDist > VIEW_DIST_FLOOR && enableFog)
				{
					uint8_t fade_ratio = (int)(perpWallDist - VIEW_DIST_FLOOR);
					floor_color >>= fade_ratio;
					celing_color >>= fade_ratio;
				}

				// floor
				ScreenSetPixel(1, x, y + verticalOffset, floor_color);

				// ceiling
				ScreenSetPixel(1, x, screenHeight - y + verticalOffset, celing_color);
			}
		}

		// timing for input and FPS counter
		oldTime = time;
		time = xTaskGetTickCount();
		portTickType frameTime = (time - oldTime) * portTICK_RATE_MS;
		sprintf(textBuffer, "FPS:%d PL:(%d,%d)", 1000 / frameTime, (int)(planeX * 57.296f), (int)(planeY * 57.296f));
		ScreenPrintStr(1, textBuffer, strlen(textBuffer), 0, 0, FONT_6x8, 15);

		// position
		sprintf(textBuffer, "X:%d Y:%d DI:(%d,%d)", (int)posX, (int)posY, (int)(dirX * 57.296f),  (int)(dirY * 57.296f));
		ScreenPrintStr(1, textBuffer, strlen(textBuffer), 0, 88, FONT_6x8, 15);

		// float frameTimeS = frameTime / 1000.0f;

		// speed modifier
		float moveSpeed = 0.2;
		float rotSpeed = 0.1;

		// get user input
		if (xQueueReceive(buttonEventQueue, &button, 0) == pdPASS)
		{
			switch (button)
			{
				case BUTTON_SELECT:
				{
					break;
				}
				case BUTTON_UP:
				{
					if (gameState == 0) break;

					float moveX = posX + dirX * moveSpeed;
					float moveY = posY + dirY * moveSpeed;

					if(worldMap[(int)(moveX)][(int)(posY)] == 0) posX = moveX;
					if(worldMap[(int)(posX)][(int)(moveY)] == 0) posY = moveY;

					break;
				}
				case BUTTON_DOWN:
				{
					if (gameState == 0) break;

					float moveX = posX - dirX * moveSpeed;
					float moveY = posY - dirY * moveSpeed;

					if(worldMap[(int)(moveX)][(int)(posY)] == 0) posX = moveX;
					if(worldMap[(int)(posX)][(int)(moveY)] == 0) posY = moveY;

					break;
				}
				case BUTTON_LEFT:
				{
					float oldDirX = dirX;
					dirX = dirX * cosf(rotSpeed) - dirY * sinf(rotSpeed);
					dirY = oldDirX * sinf(rotSpeed) + dirY * cosf(rotSpeed);
					float oldPlaneX = planeX;
					planeX = planeX * cosf(rotSpeed) - planeY * sinf(rotSpeed);
					planeY = oldPlaneX * sinf(rotSpeed) + planeY * cosf(rotSpeed);
					break;
				}
				case BUTTON_RIGHT:
				{
					float oldDirX = dirX;
					dirX = dirX * cosf(-rotSpeed) - dirY * sinf(-rotSpeed);
					dirY = oldDirX * sinf(-rotSpeed) + dirY * cosf(-rotSpeed);
					float oldPlaneX = planeX;
					planeX = planeX * cosf(-rotSpeed) - planeY * sinf(-rotSpeed);
					planeY = oldPlaneX * sinf(-rotSpeed) + planeY * cosf(-rotSpeed);
					break;
				}
				default:
				{
					break;
				}
			}
		}

		// we don't care if we successfully give the semaphore or not since we only need to
		// update the screen once no matter how many write operations applied to the screen
		// buffer
		xSemaphoreGive(screenUpdateEvent);

		// run this task at precisely at 100Hz
		vTaskDelayUntil(&xLastWakeTime, (50 / portTICK_RATE_MS));
	}
}




void ScreenUpdateThread( void *args )
{
	// pass argument
	xSemaphoreHandle screenUpdateEvent = (xSemaphoreHandle) args;

	while (1)
	{
		// block until other task grant the permission drawing screen
		xSemaphoreTake(screenUpdateEvent, portMAX_DELAY);

		// mix three layers of framebuffer and display them on screen
		ScreenUpdate();
	}
}

void TimerUpdateThread(void *args)
{
	// pass argument
	xSemaphoreHandle screenUpdateEvent = (xSemaphoreHandle) args;

	portTickType xLastWakeTime = xTaskGetTickCount();

	char buffer[16];
	long counter = 0;

	while (1)
	{
		memset(buffer, 0x0, sizeof(buffer));
		sprintf(buffer, "count:%d", counter++);
		ScreenClearFrameBuffer(2);
		ScreenPrintStr(2, buffer, strlen(buffer), 50, 50, FONT_6x8, 7);
		//ScreenDrawBox(2, 50, 50, 70, 70, 7);

		// execute this task at 1 hz
		vTaskDelayUntil(&xLastWakeTime, (1000 / portTICK_RATE_MS));
	}
}

/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	/* This function will only be called if an API call to create a task, queue
	or semaphore fails because there is too little heap RAM remaining. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed char *pcTaskName )
{
	/* This function will only be called if a task overflows its stack.  Note
	that stack overflow checking does slow down the context switch
	implementation. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
	/* This example does not use the idle hook to perform any processing. */
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
	/* This example does not use the tick hook to perform any processing. */
}


