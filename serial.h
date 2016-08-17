/*
 * serial.h
 *
 *  Created on: 15/08/2016
 *      Author: rba90
 */

#ifndef SERIAL_H_
#define SERIAL_H_

// initialize serial interface on PA1 and PA2
void SerialInit();

// serial handler thread that send an receive bytes over
// serial with other player
void SerialHandlerThread( void *args );



#endif /* SERIAL_H_ */
