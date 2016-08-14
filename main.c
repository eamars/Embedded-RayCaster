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
#include "sfx.h"

#include "raycaster.h"
#include "texture.h"
#include "world_map.h"


/* Used as a loop counter to create a very crude delay. */
#define mainDELAY_LOOP_COUNT		( 0xfffff )


/* The task function. */
void ScreenUpdateThread( void *args );
void CountdownTimerThread(void *args);

Player_t currentPlayer;
Settings_t gameSettings;

typedef struct
{
	float x;
	float y;
	uint8_t texture;
} Sprite_t;


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

void ConfigInit()
{
	// player settings
	currentPlayer.posX = 22.0f;
	currentPlayer.posY = 11.5f;
	currentPlayer.dirX = -1.0f;
	currentPlayer.dirY = 0.0f;
	currentPlayer.planeX = 0.0f;
	currentPlayer.planeY = 0.66f;

	// game settings
	gameSettings.renderFog = true;
	gameSettings.renderFloor = false;
	gameSettings.enableSFX = true;
}


int main( void ){
	/* Set the clocking to run from the PLL at 50 MHz.  Assumes 8MHz XTAL,
	whereas some older eval boards used 6MHz. */
	SysCtlClockSet( SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ );

	// Initialize peripherials
	PinReset();
	PinInit();

	ScreenInit();
	ButtonPollingInit();

	SFXInit(SysCtlClockGet());

	// create queue for sfx event
	xQueueHandle sfxEventQueue = xQueueCreate(5, sizeof(uint8_t));

	// create semaphore for screen update event
	xSemaphoreHandle screenUpdateEvent;
	xQueueHandle buttonUpdateEventQueue;

	vSemaphoreCreateBinary(screenUpdateEvent);
	buttonUpdateEventQueue = xQueueCreate(2, sizeof(uint8_t));

	// create an argument handler that pass multiple argument to tasks
	ArgumentHandler mainThreadArgumentHandler = {.arg0 = buttonUpdateEventQueue, .arg1 = screenUpdateEvent};
	ArgumentHandler buttonThreadArgumentHandler = {.arg0 = buttonUpdateEventQueue, .arg1 = sfxEventQueue};

	xTaskCreate(RayCaster, "RayCaster", 1024, (void *) &mainThreadArgumentHandler, 1, NULL);
	xTaskCreate(ScreenUpdateThread, "ScreenUpdateThread", 48, (void *) screenUpdateEvent, 2, NULL);
	xTaskCreate(SFXPlayerThread, "SFXPlayerThread", 48, (void *) sfxEventQueue, 1, NULL);
	xTaskCreate(ButtonPoll, "ButtonPoll", 96, (void *) &buttonThreadArgumentHandler, 4, NULL);

	// initialize game settings
	ConfigInit();

	/* Start the scheduler so our tasks start executing. */
	vTaskStartScheduler();	
	
	/* If all is well we will never reach here as the scheduler will now be
	running.  If we do reach here then it is likely that there was insufficient
	heap available for the idle task to be created. */
	for( ;; );
}
/*-----------------------------------------------------------*/



void takeScreenShot()
{
	// enter software critical section
	portENTER_CRITICAL();

	// scan through pixels in framebuffer[0] and print on screen
	uint8_t i, j;

	for (i = 0; i < 96; i++)
	{
		for (j = 0; j < 128; j++)
		{
			printf("0x%x, ", ScreenGetPixel(0, j, i));
		}
	}

	// exit software critical section
	portENTER_CRITICAL();
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

	xQueueHandle buttonUpdateEventQueue = (xQueueHandle) argumentHandler->arg0;
	xSemaphoreHandle screenUpdateEvent = (xSemaphoreHandle) argumentHandler->arg1;

	// initialize the task tick handler
	portTickType xLastWakeTime;

	// measure framerate
	portTickType time = 0;
	portTickType oldTime = 0;

	// iterative variable
	int x, y;
	int i;

	// string buffer
	char textBuffer[32];

	// zbuffer for storing sprites
	float zbuffer[screenWidth];

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

	// initialize the task tick handler
	xLastWakeTime = xTaskGetTickCount();

	while (1)
	{
		ScreenClearFrameBuffer(1);

		for (x = 0; x < screenWidth; x++)
		{
			// calculate ray position and direction
			float cameraX = 2 * x / (float) screenWidth - 1; 		// x coordinate in camera space
			float rayPosX = currentPlayer.posX;
			float rayPosY = currentPlayer.posY;
			float rayDirX = currentPlayer.dirX + currentPlayer.planeX * cameraX;
			float rayDirY = currentPlayer.dirY + currentPlayer.planeY * cameraX;

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
			// since the world map is a squre, the max view distance is
			for (i = 0; i < adjDist; i++)
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
				if (worldMap[mapX][mapY] > 0)
				{
					hit = 1;
					break;
				}
			}

			// if ray cannot reach otherside, then quit
			if (!hit) continue;


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

				if (perpWallDist > VIEW_DIST_WALL && gameSettings.renderFog)
				{
					color >>= (int)(perpWallDist - VIEW_DIST_WALL);
				}

				// set pixel
				ScreenSetPixel(1, x, y + verticalOffset, color);
			}

			// set the zbuffer for sprite casting
			zbuffer[x] = perpWallDist;

			/*
			// floor casting
			if (gameSettings.renderFloor)
			{
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

				// draw the floor from drawEnd to the bottom of the screen
				for (y = drawEnd; y < screenHeight; y++)
				{
					// currentDist = screenHeight / (2.0f * y - screenHeight);	// small lookup table can be used instead
					currentDist = currentDistTable[y - screenHeight / 2];

					float weight = currentDist / distWall;

					float currentFloorX = weight * floorXWall + (1.0f - weight) * currentPlayer.posX;
					float currentFloorY = weight * floorYWall + (1.0f - weight) * currentPlayer.posY;

					int floorTexX, floorTexY;
					floorTexX = ((int) (currentFloorX * texWidth)) % texWidth;
					floorTexY = ((int) (currentFloorY * texHeight)) % texHeight;

					uint8_t floor_color = texture[TEXTURE_STONE_BRICK][texWidth * floorTexY + floorTexX] >> 1;
					uint8_t celing_color = texture[TEXTURE_GREYSTONE][texWidth * floorTexY + floorTexX];

					if (currentDist > VIEW_DIST_FLOOR && gameSettings.renderFog)
					{
						uint8_t fade_ratio = (int)(perpWallDist - VIEW_DIST_FLOOR);
						floor_color >>= fade_ratio;
						celing_color >>= fade_ratio;
					}

					// floor
					ScreenSetPixel(1, x, y + verticalOffset, floor_color);

					// ceiling
					ScreenSetPixel(1, x, screenHeight - y - 1 + verticalOffset, celing_color);
				}
			}
			 */


			// sprite casting
			// assume there is a sprite at (20,10)
			Sprite_t sprite = {.x = 20.0f, .y = 10.0f, .texture = 2};

			// take square distance
			/*
			float spriteDistance = (
					(currentPlayer.posX - sprite.x) * (currentPlayer.posX - sprite.x) +
					(currentPlayer.posY - sprite.y) * (currentPlayer.posY - sprite.y)
			);
			*/

			// do the projection and draw item
			float spriteX = sprite.x - currentPlayer.posX;
			float spriteY = sprite.y - currentPlayer.posY;

			//transform sprite with the inverse camera matrix
			  // [ planeX   dirX ] -1                                       [ dirY      -dirX ]
			  // [               ]       =  1/(planeX*dirY-dirX*planeY) *   [                 ]
			  // [ planeY   dirY ]                                          [ -planeY  planeX ]
			float invDet = 1.0f / (currentPlayer.planeX * currentPlayer.dirY - currentPlayer.dirX * currentPlayer.planeY);
			float transformX = invDet * (currentPlayer.dirY * spriteX - currentPlayer.dirX * spriteY);
			float transformY = invDet * (-currentPlayer.planeY * spriteX + currentPlayer.planeX * spriteY);

			int spriteScreenX = (int)((screenWidth / 2 * (1 + transformX / transformY)));
			// scalling and moving the sprite
#define uDiv 1
#define vDiv 1
#define vMove 0.0f

			int vMoveScreen = (int) (vMove / transformY);

			// calculate height of the sprite on screen
			int spriteHeight = abs((int)(screenHeight / transformY));

			// calculate lowest and highest pixel to fill in current stripe
			int drawStartY = -spriteHeight / 2 + screenHeight / 2 + vMoveScreen;
			if (drawStart < 0) drawStartY = 0;

			int drawEndY = spriteHeight / 2 + screenHeight / 2 + vMoveScreen;
			if (drawEnd >= screenHeight) drawEndY = screenHeight - 1;

			// calculate width of the sprite
			int spriteWidth = abs((int)(screenHeight / transformY));

			int drawStartX = -spriteWidth / 2 + spriteScreenX;
			if (drawStartX < 0) drawStartX = 0;

			int drawEndX = spriteWidth / 2 + spriteScreenX;
			if (drawEndX >= screenWidth) drawEndX = screenWidth - 1;

			// loop through every vertical stripe of the sprite on screen
			// alone with vertical scan line
			if (x >= drawStartX && x < drawEndX)
			{
				int texX = (int)(256 * (x - (-spriteWidth / 2 + spriteScreenX)) * texWidth / spriteWidth) / 256;
				if (transformY > 0 && x > 0 && x < screenWidth && transformY < zbuffer[x])
				{
					for (y = drawStartY; y < drawEndY; y++)
					{
						int d = (y - vMoveScreen) * 256 - screenHeight * 128 + spriteHeight * 128;
						int texY = ((d * texHeight) / spriteHeight) / 256;

						uint8_t color = texture[sprite.texture][texWidth * texY + texX];
						ScreenSetPixel(1, x, y, color);
					}
				}
			}

		}

		// timing for input and FPS counter
		oldTime = time;
		time = xTaskGetTickCount();
		portTickType frameTime = (time - oldTime) * portTICK_RATE_MS;
		sprintf(textBuffer, "FPS:%d PL:(%d,%d)", 1000 / frameTime, (int)(currentPlayer.planeX * 57.296f), (int)(currentPlayer.planeY * 57.296f));
		ScreenPrintStr(1, textBuffer, strlen(textBuffer), 0, 0, FONT_6x8, 15);

		// position
		sprintf(textBuffer, "X:%d Y:%d DI:(%d,%d)", (int)currentPlayer.posX, (int)currentPlayer.posY, (int)(currentPlayer.dirX * 57.296f),  (int)(currentPlayer.dirY * 57.296f));
		ScreenPrintStr(1, textBuffer, strlen(textBuffer), 0, 88, FONT_6x8, 15);

		// we don't care if we successfully give the semaphore or not since we only need to
		// update the screen once no matter how many write operations applied to the screen
		// buffer
		xSemaphoreGive(screenUpdateEvent);

		// wait for user input
		uint8_t button_type;
		xQueueReceive(buttonUpdateEventQueue, &button_type, portMAX_DELAY);

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


