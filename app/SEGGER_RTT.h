/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*       Solutions for real time microcontroller applications         *
**********************************************************************
*                                                                    *
*            (c) 1995 - 2024 SEGGER Microcontroller GmbH             *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
*                                                                    *
*       SEGGER RTT * Real Time Transfer for embedded targets         *
*                                                                    *
**********************************************************************
*
* Simplified RTT implementation for Alif E8 MNIST Demo
*/

#ifndef SEGGER_RTT_H
#define SEGGER_RTT_H

#include <stdint.h>
#include <stdarg.h>

/*********************************************************************
* RTT API
*/
void SEGGER_RTT_Init(void);
unsigned SEGGER_RTT_Write(unsigned BufferIndex, const void* pBuffer, unsigned NumBytes);
unsigned SEGGER_RTT_WriteString(unsigned BufferIndex, const char* s);
int SEGGER_RTT_printf(unsigned BufferIndex, const char* sFormat, ...);
int SEGGER_RTT_HasKey(void);
int SEGGER_RTT_GetKey(void);

#endif /* SEGGER_RTT_H */
