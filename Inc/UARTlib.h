/*
 * UARTlib.h
 *
 *  Created on: Nov 1, 2022
 *      Author: archero
 */

#ifndef SRC_UARTLIB_H_
#define SRC_UARTLIB_H_

// needed for uart struct
#include "stm32g4xx_hal.h"

/// Binds callback to UART ID.
///
/// Specify ID and callback for frame.
void SetUARTCallback(uint8_t ID, void (*CALLBACK)(uint8_t LEN, uint8_t * payload));


/// Ends connection.
///
/// Deallocates memory.
void CloseUART();

/// Initializes library.
///
/// Must be used before any other library call.
void InitUART(UART_HandleTypeDef * uart_struct);


#endif /* SRC_UARTLIB_H_ */
