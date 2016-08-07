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

#define mapWidth 24
#define mapHeight 24

#define screenWidth 128
#define screenHeight 96

typedef struct
{
	void *arg0;
	void *arg1;
	void *arg2;
	void *arg3;
} ArgumentHandler;


/* Used as a loop counter to create a very crude delay. */
#define mainDELAY_LOOP_COUNT		( 0xfffff )

int worldMap[mapWidth][mapHeight]=
{
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,4,4,4,4,4,0,0,0,0,4,0,4,0,4,0,0,0,1},
	{1,0,0,0,0,0,4,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,4,0,0,0,4,0,0,0,0,4,0,0,0,4,0,0,0,1},
	{1,0,0,0,0,0,4,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,4,4,0,4,4,0,0,0,0,4,0,4,0,4,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};


/* The task function. */
void RayCaster(void *args);
void ScreenUpdateThread( void *args );



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



int main( void )
{
	/* Set the clocking to run from the PLL at 50 MHz.  Assumes 8MHz XTAL,
	whereas some older eval boards used 6MHz. */
	SysCtlClockSet( SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ );

	// create queue for button event passing
	xQueueHandle buttonEventQueue = xQueueCreate(5, sizeof(uint8_t));

	// create semaphore for screen update event
	xSemaphoreHandle screenUpdateEvent = xSemaphoreCreateCounting(10, 1);

	// create an argument handler that pass multiple argument to tasks
	ArgumentHandler handler = {.arg0 = buttonEventQueue, .arg1 = screenUpdateEvent};

	// Initialize peripherials
	PinReset();
	PinInit();

	ScreenInit();
	ButtonInit(buttonEventQueue);


	xTaskCreate(RayCaster, "RayCaster", 2000, (void *) &handler, 1, NULL);
	xTaskCreate(ScreenUpdateThread, "ScreenUpdateThread", 240, (void *) screenUpdateEvent, 4, NULL);

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



void RayCaster(void *args)
{
	// pass argument
	ArgumentHandler *argumentHandler = (ArgumentHandler *) args;

	xQueueHandle buttonEventQueue = (xQueueHandle) argumentHandler->arg0;
	xSemaphoreHandle screenUpdateEvent = (xSemaphoreHandle) argumentHandler->arg1;

	// initialize the task tick handler
	portTickType xLastWakeTime;

	// string
	char textBuffer[16];

	// button
	int8_t button;

	// variables about ray casting
	float posX = 22;
	float posY = 12; 		// starting position
	float dirX = -1;
	float dirY = 0;		// initial direction vector
	float planeX = 0;
	float planeY = 0.66;	// 2d raycaster version of camera plane

	// measure framerate
	portTickType time = 0;
	portTickType oldTime = 0;

	int x;

	// initialize the task tick handler
	xLastWakeTime = xTaskGetTickCount();

	while (1)
	{
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
			float deltaDistX = sqrt(1 + (rayDirY * rayDirY) / (rayDirX * rayDirX));
			float deltaDistY = sqrt(1 + (rayDirX * rayDirX) / (rayDirY * rayDirY));
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
				sideDistX = (mapX + 1.0 - rayPosX) * deltaDistX;
			}

			if (rayDirY < 0)
			{
				stepY = -1;
				sideDistY = (rayPosY - mapY) * deltaDistY;
			}
			else
			{
				stepY = 1;
				sideDistY = (mapY + 1.0 - rayPosY) * deltaDistY;
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

			// choose wall color
			// simulate color by gray level
			uint8_t color;
			switch(worldMap[mapX][mapY])
			{
				case 1: color = 2; break;
				case 2: color = 4; break;
				case 3: color = 6; break;
				case 4: color = 8; break;
				default: color = 10; break;
			}

			// give x and y sides different brightness
			if (side == 1)
			{
				color /= 2;
			}

			// draw the pixels of the stripes as a vertical line
			ScreenDrawLine(1, x, drawStart, x, drawEnd, color);

		}

		// timing for input and FPS counter
		oldTime = time;
		time = xTaskGetTickCount();
		portTickType frameTime = (time - oldTime) * portTICK_RATE_MS;
		sprintf(textBuffer, "FPS:%d", 1000 / frameTime);
		ScreenDrawBox(1, 0, 0, 127, 8, 0);
		ScreenPrintStr(1, textBuffer, strlen(textBuffer), 0, 0, FONT_6x8, 10);

		float frameTimeS = frameTime / 1000.0f;

		// speed modifier
		float moveSpeed = frameTimeS * 5.0;
		float rotSpeed = frameTimeS * 3.0;

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
					if(worldMap[(int)(posX + dirX * moveSpeed)][(int)(posY)] == false) posX += dirX * moveSpeed;
					if(worldMap[(int)(posX)][(int)(posY + dirY * moveSpeed)] == false) posY += dirY * moveSpeed;

					break;
				}
				case BUTTON_DOWN:
				{
					if(worldMap[(int)(posX + dirX * moveSpeed)][(int)(posY)] == false) posX -= dirX * moveSpeed;
					if(worldMap[(int)(posX)][(int)(posY + dirY * moveSpeed)] == false) posY -= dirY * moveSpeed;

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


