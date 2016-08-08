/*
 * texture.h
 *
 *  Created on: 9/08/2016
 *      Author: rba90
 */

#ifndef TEXTURE_H_
#define TEXTURE_H_

#include <stdint.h>

#define texWidth 16
#define texHeight 16

uint8_t ence463_block[texWidth * texHeight] = {
		10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
		10, 5 , 5 , 5 , 10, 5 , 10, 5 , 10, 10, 5 , 5 , 10, 5 , 5 , 5 ,
		10, 5 , 10, 10, 10, 5 , 5 , 5 , 10, 5 , 10, 10, 10, 5 , 10, 10,
		10, 5 , 5 , 5 , 10, 5 , 5 , 5 , 10, 5 , 10, 10, 10, 5 , 5 , 5 ,
		10, 5 , 10, 10, 10, 5 , 10, 5 , 10, 5 , 10, 10, 10, 5 , 10, 10,
		10, 5 , 5 , 5 , 10, 5 , 10, 5 , 10, 10, 5 , 5 , 10, 5 , 5 , 5 ,
		10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
		10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
		10, 5 , 10, 5 , 10, 10, 5 , 5 , 10, 5 , 5 , 10, 10, 10, 10, 10,
		10, 5 , 10, 5 , 10, 5 , 10, 10, 10, 10, 10, 5 , 10, 10, 10, 10,
		10, 5 , 5 , 5 , 10, 5 , 5 , 5 , 10, 5 , 5 , 10, 10, 10, 10, 10,
		10, 10, 10, 5 , 10, 5 , 10, 5 , 10, 10, 10, 5 , 10, 10, 10, 10,
		10, 10, 10, 5 , 10, 5 , 5 , 5 , 10, 5 , 5 , 10, 10, 10, 10, 10,
		10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
		10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
		10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10
};

uint8_t stone_brick[texWidth * texHeight] = {
		15, 13, 13, 13, 15, 15, 15, 15, 13, 13, 13, 13, 13, 13, 15, 13,
		13, 11, 13, 13, 13, 13, 11, 11, 11, 11, 11, 11, 13, 13, 13, 6,
		13, 13, 11, 13, 13, 11, 11, 11, 11, 11, 13, 13, 13, 13, 11, 4,
		13, 13, 11, 11, 13, 11, 13, 13, 11, 13, 13, 11, 11, 11, 11, 6,
		13, 11, 11, 11, 13, 13, 11, 11, 11, 11, 13, 11, 11, 13, 11, 6,
		13, 13, 13, 13, 11, 11, 11, 11, 11, 13, 13, 11, 13, 11, 11, 6,
		15, 13, 13, 11, 11, 11, 13, 13, 13, 11, 11, 11, 11, 11, 11, 6,
		11, 6, 6, 4, 6, 6, 6, 6, 6, 6, 4, 6, 6, 6, 6, 4,
		13, 15, 15, 13, 15, 15, 13, 11, 15, 13, 13, 15, 15, 13, 13, 13,
		11, 11, 13, 13, 13, 11, 11, 6, 13, 11, 13, 13, 13, 11, 11, 11,
		13, 13, 11, 11, 13, 11, 11, 6, 15, 13, 13, 11, 11, 11, 13, 11,
		13, 11, 11, 11, 11, 13, 13, 6, 15, 11, 11, 11, 13, 13, 13, 11,
		11, 11, 11, 13, 13, 11, 13, 6, 13, 11, 13, 13, 13, 11, 13, 13,
		11, 11, 11, 13, 13, 13, 11, 6, 13, 13, 13, 11, 13, 13, 11, 11,
		11, 11, 13, 13, 11, 11, 11, 6, 13, 11, 13, 13, 11, 11, 11, 11,
		6, 6, 6, 4, 4, 6, 6, 4, 11, 6, 6, 6, 6, 6, 6, 6
};

uint8_t stone_brick_carved[texWidth * texHeight] =
{
		15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 13, 15, 15, 15, 13,
		13, 11, 13, 11, 11, 11, 11, 11, 11, 9, 11, 11, 11, 11, 11, 6,
		8, 6, 6, 6, 6, 4, 4, 4, 6, 6, 6, 8, 6, 8, 11, 4,
		15, 15, 15, 13, 15, 15, 15, 15, 15, 15, 15, 15, 11, 13, 9, 6,
		15, 11, 9, 11, 11, 11, 11, 11, 11, 11, 11, 9, 4, 15, 11, 8,
		13, 11, 6, 8, 6, 4, 4, 4, 6, 6, 13, 11, 6, 13, 11, 4,
		15, 11, 6, 13, 15, 13, 15, 15, 15, 6, 13, 11, 6, 13, 11, 8,
		15, 11, 4, 13, 6, 6, 9, 6, 13, 4, 13, 11, 6, 15, 11, 6,
		13, 11, 9, 15, 6, 9, 11, 13, 11, 6, 15, 11, 9, 15, 11, 4,
		15, 11, 8, 15, 4, 6, 4, 6, 6, 6, 15, 13, 6, 13, 11, 4,
		15, 11, 6, 15, 13, 15, 15, 15, 15, 15, 13, 11, 4, 13, 11, 6,
		15, 11, 6, 13, 11, 11, 11, 11, 11, 11, 11, 9, 6, 15, 11, 6,
		13, 11, 4, 11, 8, 6, 6, 6, 4, 6, 6, 8, 8, 15, 11, 6,
		13, 11, 9, 15, 15, 15, 13, 13, 13, 15, 15, 15, 15, 13, 11, 4,
		15, 11, 13, 11, 11, 11, 11, 11, 11, 11, 13, 11, 11, 9, 9, 6,
		11, 9, 6, 4, 4, 6, 6, 6, 8, 8, 6, 4, 4, 6, 6, 6
};

uint8_t nether_brick[texWidth * texHeight] =
{
		4, 4, 4, 2, 2, 4, 4, 4, 4, 4, 2, 2, 2, 6, 4, 4,
		4, 2, 4, 2, 4, 4, 4, 4, 4, 4, 4, 2, 6, 4, 2, 4,
		2, 2, 4, 0, 4, 4, 4, 2, 2, 4, 4, 0, 4, 4, 4, 2,
		2, 2, 0, 2, 4, 4, 4, 0, 0, 2, 0, 2, 4, 4, 4, 0,
		2, 6, 4, 4, 4, 4, 2, 2, 2, 6, 4, 4, 4, 4, 2, 2,
		6, 2, 4, 4, 2, 4, 2, 0, 6, 4, 4, 4, 4, 4, 4, 0,
		4, 2, 4, 2, 4, 2, 2, 0, 2, 4, 4, 2, 2, 2, 2, 2,
		2, 0, 2, 0, 0, 2, 2, 2, 2, 2, 0, 0, 2, 2, 2, 2,
		4, 4, 2, 2, 2, 4, 4, 4, 4, 4, 2, 2, 2, 6, 4, 4,
		4, 4, 4, 2, 4, 2, 4, 4, 4, 4, 2, 2, 6, 2, 4, 4,
		2, 4, 4, 0, 4, 4, 4, 4, 2, 4, 0, 2, 4, 4, 4, 2,
		2, 2, 0, 2, 4, 4, 4, 2, 2, 0, 2, 2, 4, 4, 4, 0,
		2, 6, 4, 4, 4, 4, 2, 0, 2, 6, 4, 4, 4, 4, 2, 0,
		6, 4, 4, 4, 4, 4, 4, 2, 6, 4, 4, 4, 2, 4, 2, 0,
		4, 4, 4, 2, 4, 2, 2, 0, 4, 2, 4, 2, 4, 2, 2, 2,
		2, 2, 0, 2, 2, 0, 0, 2, 2, 2, 0, 2, 2, 2, 0, 2
};

uint8_t brick[texWidth * texHeight] =
{
		3, 4, 4, 2, 4, 3, 4, 4, 3, 2, 4, 4, 3, 3, 3, 3,
		4, 4, 3, 2, 5, 4, 4, 4, 4, 2, 4, 4, 4, 4, 4, 3,
		2, 2, 2, 1, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2,
		5, 4, 4, 4, 3, 5, 5, 4, 4, 4, 4, 4, 5, 2, 5, 5,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 4, 3,
		3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 2,
		4, 4, 4, 4, 4, 4, 3, 3, 4, 4, 4, 3, 4, 4, 4, 2,
		3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		5, 5, 3, 3, 5, 4, 4, 4, 4, 4, 3, 4, 5, 4, 4, 4,
		3, 3, 2, 3, 4, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 3,
		3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 2,
		4, 4, 4, 4, 3, 4, 4, 3, 3, 4, 3, 4, 3, 4, 4, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		4, 4, 4, 4, 2, 4, 4, 4, 4, 4, 4, 3, 4, 4, 4, 4,
		4, 3, 4, 4, 2, 4, 4, 3, 3, 3, 3, 2, 3, 3, 4, 3,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2
};

uint8_t greystone[texWidth * texHeight] =
{
		2, 2, 2, 5, 5, 5, 3, 3, 4, 4, 2, 3, 3, 3, 2, 2,
		5, 4, 3, 5, 5, 4, 3, 3, 4, 4, 1, 4, 5, 5, 5, 5,
		3, 2, 2, 5, 4, 4, 3, 1, 2, 2, 2, 4, 4, 4, 3, 3,
		1, 1, 2, 5, 4, 4, 3, 2, 5, 5, 2, 2, 1, 1, 1, 1,
		5, 4, 2, 4, 4, 3, 2, 2, 4, 3, 2, 4, 3, 5, 5, 5,
		4, 4, 1, 3, 2, 1, 2, 3, 3, 3, 1, 5, 2, 5, 4, 4,
		3, 2, 1, 5, 4, 2, 5, 5, 5, 4, 2, 5, 2, 2, 2, 3,
		3, 4, 3, 5, 4, 2, 5, 4, 4, 4, 2, 5, 2, 3, 3, 2,
		5, 4, 3, 5, 4, 2, 5, 4, 4, 3, 2, 5, 3, 4, 4, 3,
		5, 4, 3, 5, 4, 1, 3, 3, 2, 1, 1, 2, 2, 2, 2, 2,
		5, 4, 2, 4, 4, 2, 2, 4, 4, 4, 5, 5, 5, 5, 5, 2,
		4, 3, 2, 4, 4, 2, 2, 5, 4, 4, 5, 4, 4, 4, 4, 2,
		2, 2, 1, 1, 1, 1, 1, 4, 3, 4, 4, 4, 4, 4, 4, 2,
		4, 5, 5, 4, 4, 4, 4, 4, 2, 1, 1, 1, 1, 1, 1, 1,
		4, 3, 3, 3, 3, 3, 3, 3, 1, 5, 5, 3, 4, 4, 4, 3,
		2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2
};

uint8_t steve_block[texWidth * texHeight] =
{
		3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 2, 1, 1, 2, 3, 3, 3, 4, 4, 3, 3, 4, 4,
		3, 3, 1, 1, 1, 2, 3, 4, 5, 5, 4, 4, 4, 4, 4, 4,
		3, 3, 1, 1, 2, 3, 4, 5, 5, 6, 5, 4, 4, 4, 4, 4,
		3, 3, 0, 1, 0, 1, 2, 4, 2, 4, 5, 4, 4, 4, 4, 4,
		3, 2, 0, 1, 2, 2, 2, 5, 4, 6, 6, 6, 4, 4, 4, 4,
		3, 2, 1, 2, 2, 2, 2, 5, 5, 6, 7, 6, 4, 4, 4, 4,
		3, 3, 3, 2, 2, 1, 2, 5, 6, 6, 6, 5, 4, 4, 4, 4,
		3, 4, 4, 2, 2, 2, 2, 5, 6, 6, 5, 4, 4, 4, 4, 4,
		3, 4, 3, 2, 2, 3, 3, 5, 6, 5, 5, 5, 4, 4, 5, 5,
		3, 3, 2, 3, 3, 4, 4, 5, 6, 3, 4, 5, 6, 5, 5, 5,
		4, 4, 3, 3, 3, 4, 5, 5, 6, 3, 5, 5, 5, 6, 6, 5,
		5, 4, 4, 4, 4, 4, 4, 4, 3, 5, 6, 6, 6, 6, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5, 4, 7, 6, 6, 6, 6, 5, 5,
		5, 6, 5, 5, 5, 6, 5, 5, 5, 6, 6, 6, 6, 6, 6, 5,
		5, 6, 5, 5, 5, 6, 5, 5, 5, 6, 6, 6, 6, 6, 6, 5
};


#endif /* TEXTURE_H_ */
