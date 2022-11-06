/*
 * UARTlib.c
 *
 *  Created on: Nov 1, 2022
 *      Author: archero
 */

#include "UARTlib.h"

// --------------------------- Implementations includes ---------------------------
#include "stm32g4xx_hal.h"
#include <string.h>
// --------------------------- End of implementations includes ---------------------------

// --------------------------- Predefinitions ---------------------------
struct __struct_UARTLIB
{
	uint8_t ID;
	void (*CALLBACK)(uint8_t LEN, uint8_t * payload);
};
void __frames_interpretation_func_UART(uint8_t ID);
// --------------------------- End of predefinitions ---------------------------


// --------------------------- Library values ---------------------------

uint8_t __control_UARTLIB = 1; //< if set to 1 then library is not initialized

struct __struct_UARTLIB * __POINTER_STRUCT_UARTLIB; //< its pointer to array of __struct_UARTLIB where is stored connection callback + di 

uint8_t __stack_size_UARTLIB = 10; //< starting size
uint8_t __current__index_UARTLIB = 0; //< points to index of currently added struct to __POINTER_STRUCT_UARTLIB

UART_HandleTypeDef * __reference_struct_UARTLIB; //< reference to uart struct provided by user in InitUART(params);

uint8_t __boilerplate_UARTLIB[3]; //< buffer for boilerplate bytes
uint8_t __payload_UARTLIB[5]; //< buffer for payload bytes (passed later to callback)
uint8_t __readbyte_UARTLIB = 0; //< number of currently read byte (if its 0 it means currenlty is reading 1 byte)

// --------------------------- End of library values ---------------------------

/// Initializes library.
///
/// Must be used before any other library call.
void InitUART(UART_HandleTypeDef * uart_struct)
{
	// Error handling
	if(!__control_UARTLIB) return;	//< checks if library is alreay initialized
	
	// Initialize array 
	__POINTER_STRUCT_UARTLIB = malloc(__stack_size_UARTLIB*sizeof(struct __struct_UARTLIB)); 
	// Sets control byte to 0 (library initialized)
	__control_UARTLIB = 0;
	
	// cache pointer supplied by user
	__reference_struct_UARTLIB = uart_struct;

	// starts waiting transmission (non-blocking mode)
	HAL_UART_Receive_IT(__reference_struct_UARTLIB, __boilerplate_UARTLIB[__readbyte_UARTLIB], 1);

}

/// Ends connection.
///
/// Deallocates memory.
void CloseUART()
{
	// Error handler
	if(__control_UARTLIB) return; //< You can't close not initialized library
	free(__POINTER_STRUCT_UARTLIB); //< free resources
	__control_UARTLIB = 1; //< make library able to anitialize again
}


/// Binds callback to UART ID.
///
/// Specify ID and callback for frame.
void SetUARTCallback(uint8_t _ID, void (*_CALLBACK)(uint8_t LEN, uint8_t * payload))
{
	// Error handler
	if(__control_UARTLIB) return; //< chceks if library is initialized

	// cheks if array should be reallocated (too small) if yes then creates bigger array by 10 (items) copies old array, deletes old.
	if(__current__index_UARTLIB >= __stack_size_UARTLIB)
	{
		__stack_size_UARTLIB += 10;
		struct __struct_UARTLIB * temp = malloc(__stack_size_UARTLIB*sizeof(struct __struct_UARTLIB));
		memcpy(temp, __POINTER_STRUCT_UARTLIB, (__stack_size_UARTLIB-10)*sizeof(struct __struct_UARTLIB));
		free(__POINTER_STRUCT_UARTLIB);
		__POINTER_STRUCT_UARTLIB = temp;
	}

	// add new callback if all conditions are met
	__POINTER_STRUCT_UARTLIB[__current__index_UARTLIB++].ID = _ID;
	__POINTER_STRUCT_UARTLIB[__current__index_UARTLIB].CALLBACK = _CALLBACK;
}


// --------------------------- Hidden functions from library's Users  ---------------------------

// User is not allowed to define this callback by himself !!!
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

	// chceks if this callback was called by uart supplied by user
	if(__reference_struct_UARTLIB != huart) return;

	__readbyte_UARTLIB++;

	if(__readbyte_UARTLIB < 3) //< if less than 3 read to boilerplate
	{
		HAL_UART_Receive_IT(__reference_struct_UARTLIB, &__boilerplate_UARTLIB[__readbyte_UARTLIB], 1);
		return;
	}
	if(__boilerplate_UARTLIB[2] <= __readbyte_UARTLIB - 2) // read to payload until max size
	{
		HAL_UART_Receive_IT(__reference_struct_UARTLIB, &__payload_UARTLIB[__readbyte_UARTLIB], 1);
		return;
	}

	__readbyte_UARTLIB = 0; // reset read bytes
	__frames_interpretation_func_UART(__boilerplate_UARTLIB[1]);

	// wait with next wait until __frames_interpretation_func_UART ends working
	HAL_UART_Receive_IT(__reference_struct_UARTLIB, &__boilerplate_UARTLIB[__readbyte_UARTLIB], 1);


}

// seraches for callback binded to given ID if found then runs it with data from curent __boilerplate_UARTLIB and __payload_UARTLIB
void __frames_interpretation_func_UART(uint8_t ID)
{
	for(int k = 0; k <= __current__index_UARTLIB; k++ )
	{
		if(__POINTER_STRUCT_UARTLIB[k].ID == ID)
		{
			__POINTER_STRUCT_UARTLIB[k].CALLBACK(__readbyte_UARTLIB - 1, __payload_UARTLIB);
		}
	}
}


