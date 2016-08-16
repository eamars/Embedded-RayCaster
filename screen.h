/*
 * screen.h
 *
 *  Created on: 28/07/2016
 *      Author: rba90
 */

#ifndef SCREEN_H_
#define SCREEN_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
	FONT_6x8 = 0,
	FONT_8x16
} Font_t;

typedef struct
{
	uint8_t x, y;
} Point_t;

// initialize the screen driver
void ScreenInit();

// erase given framebuffer
void ScreenClearFrameBuffer(uint8_t fb_idx);

// print half of screen
void ScreenPaintWithBackground(uint8_t fb_idx, uint8_t color1, uint8_t color2);

// draw a dot on screen
// for frequent used function, it is suggested to use no more than 4 arguments
// ref: http://stackoverflow.com/questions/26515261/why-does-arm-documentation-recommend-having-only-4-function-arguments-is-there
bool ScreenSetPixel(uint8_t fb_idx, uint8_t cx, uint8_t cy, uint8_t level);

// get the value of a dot on screen
uint8_t ScreenGetPixel(uint8_t fb_idx, uint8_t cx, uint8_t cy);

// shift content on a given screen buffer to the left
void ScreenShiftLeft(uint8_t fb_idx);

// shift content on a given screen buffer to the right
void ScreenShiftRight(uint8_t fb_idx);

/*
 * Using midpoint circle algorithm
 * Ref: https://en.wikipedia.org/wiki/Midpoint_circle_algorithm
 */
void ScreenDrawCircle(uint8_t fb_idx, uint8_t cx, uint8_t cy, uint8_t radius, uint8_t level);
/*
 * Using Bresenham's line algorithm
 * Ref: https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
 */
void ScreenDrawLine(uint8_t fb_idx, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t level);

// draw a rectangular area
void ScreenDrawBox(uint8_t fb_idx, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t level);

// Print a character on screen using 8x16 font
void ScreenPrintChar8x16(uint8_t fb_idx, uint8_t ch, uint8_t cx, uint8_t cy, uint8_t level);

// Print a character on screen using 6x8 font
void ScreenPrintChar6x8(uint8_t fb_idx, uint8_t ch, uint8_t cx, uint8_t cy, uint8_t level);

// Print a string on screen
void ScreenPrintStr(uint8_t fb_idx, char *str, uint8_t len, uint8_t cx, uint8_t cy, Font_t font, uint8_t level);

// clear the value of pixels in a column
void ScreenClearColumn(uint8_t fb_idx, uint8_t col);

// clear the value of pixels in a row
void ScreenClearRow(uint8_t fb_idx, uint8_t row);

// mix two layers of pixels and display them on screen
void ScreenUpdate();



#endif /* SCREEN_H_ */
