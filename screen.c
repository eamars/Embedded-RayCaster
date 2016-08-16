/*
 * screen.c
 *
 *  Created on: 28/07/2016
 *      Author: rba90
 */


#include "screen.h"
#include "font_6x8.h"
#include "drivers/rit128x96x4.h"

#include <string.h>
#include <stdint.h>

#define sign(x) ((x>0)?1:((x<0)?-1:0))

static uint8_t framebuffer[3][6144];

void ScreenClearFrameBuffer(uint8_t fb_idx)
{
	memset(framebuffer[fb_idx], 0x00, sizeof(framebuffer[fb_idx]));
}

void ScreenPaintWithBackground(uint8_t fb_idx, uint8_t color1, uint8_t color2)
{
	memset(framebuffer[fb_idx], color1, sizeof(framebuffer[fb_idx]) / 2);
	memset(framebuffer[fb_idx] + sizeof(framebuffer[fb_idx]) / 2, color2, sizeof(framebuffer[fb_idx]) / 2);

}

bool ScreenSetPixel(uint8_t fb_idx, uint8_t cx, uint8_t cy, uint8_t level)
{
	if (cx > 128 || cy > 96)
	{
		return false;
	}

	long idx = (long) cy * 64 + (long) cx / 2;

	if (cx & 1)
	{
		framebuffer[fb_idx][idx] = (framebuffer[fb_idx][idx] & 0xf0) |
						   (level & 0xf);
	}
	else
	{
		framebuffer[fb_idx][idx] = (framebuffer[fb_idx][idx] & 0x0f) |
						   ((level & 0xf) << 4);
	}

	return true;
}

uint8_t ScreenGetPixel(uint8_t fb_idx, uint8_t cx, uint8_t cy)
{
	if (cx > 128 || cy > 96)
	{
		return 0;
	}

	long idx = (long) cy * 64 + (long) cx / 2;

	if (cx & 1)
	{
		return (framebuffer[fb_idx][idx] & 0x0f);
	}
	else
	{
		return (framebuffer[fb_idx][idx] & 0xf0) >> 4;
	}
}

void ScreenShiftLeft(uint8_t fb_idx)
{
	uint8_t pixel;

	int i, j;

	for (i = 0; i < 96; i++)
	{
		for (j = 0; j < 127; j++)
		{
			pixel = ScreenGetPixel(fb_idx, j + 1, i);

			ScreenSetPixel(fb_idx, j, i, pixel);
		}

		// clear the last column of the image
		ScreenSetPixel(fb_idx, 127, i, 0);
	}
}

void ScreenShiftRight(uint8_t fb_idx)
{
	uint8_t pixel;

	int i, j;

	for (i = 96; i > 0; i--)
	{
		for (j = 127; j > 0; j--)
		{
			pixel = ScreenGetPixel(fb_idx, j - 1, i);

			ScreenSetPixel(fb_idx, j, i, pixel);
		}

		// clear the last column of the image
		ScreenSetPixel(fb_idx, 0, i, 0);
	}
}

void ScreenDrawCircle(uint8_t fb_idx, uint8_t cx, uint8_t cy, uint8_t radius, uint8_t level)
{
	int x = radius;
	int y = 0;
	int err = 0;

	ScreenSetPixel(fb_idx, cx, cy, level);
	while (x >= y)
	{
		ScreenSetPixel(fb_idx, cx + x, cy + y, level);
		ScreenSetPixel(fb_idx, cx + y, cy + x, level);
		ScreenSetPixel(fb_idx, cx - y, cy + x, level);
		ScreenSetPixel(fb_idx, cx - x, cy + y, level);
		ScreenSetPixel(fb_idx, cx - x, cy - y, level);
		ScreenSetPixel(fb_idx, cx - y, cy - x, level);
		ScreenSetPixel(fb_idx, cx + y, cy - x, level);
		ScreenSetPixel(fb_idx, cx + x, cy - y, level);

		y += 1;
		err += 1 + 2 * y;
		if (2 * (err - x) + 1 > 0)
		{
			x -= 1;
			err += 1 - 2 * x;
		}
	}
}

void ScreenDrawLine(uint8_t fb_idx, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t level)
{
	int dx, dy, x, y, d, s1, s2, swap=0, temp;

	dx = abs(x1 - x0);
	dy = abs(y1 - y0);
	s1 = sign(x1 - x0);
	s2 = sign(y1 - y0);

	if (dy > dx)
	{
		temp = dx;
		dx = dy;
		dy = temp;
		swap = 1;
	}

	d = 2 * dy - dx;
	x = x0;
	y = y0;

	int i;
	for (i = 1; i <= dx; i++)
	{
		ScreenSetPixel(fb_idx, x, y, level);

		while (d >= 0)
		{
			if (swap)
			{
				x = x + s1;
			}
			else
			{
				y = y + s2;
				d = d - 2 * dx;
			}
		}
		if (swap)
		{
			y = y + s2;
		}
		else
		{
			x = x + s1;
		}
		d = d + 2 * dy;
	}
}

void ScreenDrawBox(uint8_t fb_idx, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t level)
{
	uint8_t i, j;

	for (i = y0; i < y1; i++)
	{
		for (j = x0; j < x1; j++)
		{
			ScreenSetPixel(fb_idx, j, i, level);
		}
	}
}


void ScreenPrintChar6x8(uint8_t fb_idx, uint8_t ch, uint8_t cx, uint8_t cy, uint8_t level)
{
	unsigned cheight   = font6x8.cheight;
	unsigned cwidth    = font6x8.cwidth;
	unsigned offset    = ch * cheight;

	unsigned i, j;

	unsigned mask = 1 << (cwidth);

	for (i = 0; i < cheight; i++)
	{
		// iterate through all 8 bit pattern
		for (j = 0; j < cwidth; j++)
		{
			// filter the bit pattern, move 1 from MSB to LSB
			//
			// if line is 0b01100010, mask = 0b10000000
			//      0th bit, j = 0, mask >> 0 = 0b10000000
			//          0b01100010
			//      &   0b10000000
			//      --------------
			//          0b00000000
			//      bit pattern at 0th bit is 0, displays ' '
			//
			//
			//      1st bit, j = 1, 0x80 >> 1 = 0b01000000
			//          0b01100010
			//      &   0b01000000
			//      --------------
			//          0b01000000
			//      bit pattern at 1st bit is 64, which is not zero, displays '*'
			if (font6x8.rundata[i + offset] & (mask >> j))
			{
				ScreenSetPixel(fb_idx, cx + j, cy + i, level);
			}
		}
	}
}

void ScreenPrintStr(uint8_t fb_idx, char *str, uint8_t len, uint8_t cx, uint8_t cy, Font_t font, uint8_t level)
{
	uint8_t offset;
	uint8_t i;

	offset = 0;

	for (i = 0; i < len; i++)
	{
		switch (font)
		{
			case FONT_6x8:
			{
				ScreenPrintChar6x8(fb_idx, str[i], offset + cx, cy, level);
				offset += 6;
				break;
			}
			default:
				break;
		}
	}
}

void ScreenClearColumn(uint8_t fb_idx, uint8_t col)
{
	uint8_t row;

	for (row = 0; row < 96; row++)
	{
		ScreenSetPixel(fb_idx, col, row, 0);
	}
}

void ScreenClearRow(uint8_t fb_idx, uint8_t row)
{
	uint8_t col;

	for (col = 0; col < 128; col++)
	{
		ScreenSetPixel(fb_idx, col, row, 0);
	}
}

void ScreenUpdate()
{
	unsigned long ulLoop;

	// blend layers
	for(ulLoop = 0; ulLoop < 6144; ulLoop += 4)
	{
		*((unsigned long *)(framebuffer[0] + ulLoop)) =
				*((unsigned long *)(framebuffer[1] + ulLoop)) | 	// use or to mix layers
				*((unsigned long *)(framebuffer[2] + ulLoop))
		;
	}

	RIT128x96x4ImageDraw(framebuffer[0], 0, 0, 128, 96);
}

void ScreenInit()
{
	// initialize the OLED driver
	RIT128x96x4Init(3500000);

	// turn the screen on
	RIT128x96x4DisplayOn();

	// initialize framebuffer
	memset(framebuffer, 0x0, sizeof(framebuffer));
}
