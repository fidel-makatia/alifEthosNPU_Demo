/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*       Solutions for real time microcontroller applications         *
**********************************************************************
* Simplified SEGGER RTT implementation for Alif E8 MNIST Demo
*
* This is a minimal implementation sufficient for printf-style output.
* For full RTT functionality, use the official SEGGER RTT package.
*********************************************************************/

#include "SEGGER_RTT.h"
#include <string.h>

/*********************************************************************
* RTT Control Block
*/
#define SEGGER_RTT_MAX_NUM_UP_BUFFERS    1
#define SEGGER_RTT_MAX_NUM_DOWN_BUFFERS  1
#define BUFFER_SIZE_UP                   1024
#define BUFFER_SIZE_DOWN                 64

typedef struct {
    char*         sName;
    char*         pBuffer;
    unsigned      SizeOfBuffer;
    unsigned      WrOff;
    volatile unsigned RdOff;
    unsigned      Flags;
} SEGGER_RTT_BUFFER_UP;

typedef struct {
    char*         sName;
    char*         pBuffer;
    unsigned      SizeOfBuffer;
    volatile unsigned WrOff;
    unsigned      RdOff;
    unsigned      Flags;
} SEGGER_RTT_BUFFER_DOWN;

typedef struct {
    char                    acID[16];
    int                     MaxNumUpBuffers;
    int                     MaxNumDownBuffers;
    SEGGER_RTT_BUFFER_UP    aUp[SEGGER_RTT_MAX_NUM_UP_BUFFERS];
    SEGGER_RTT_BUFFER_DOWN  aDown[SEGGER_RTT_MAX_NUM_DOWN_BUFFERS];
} SEGGER_RTT_CB;

/*********************************************************************
* Static data
*/
static char _acUpBuffer[BUFFER_SIZE_UP];
static char _acDownBuffer[BUFFER_SIZE_DOWN];

/* RTT Control Block - must be found by J-Link */
__attribute__((section(".rtt_cb"), used))
static SEGGER_RTT_CB _SEGGER_RTT = {
    .acID = "SEGGER RTT",
    .MaxNumUpBuffers = SEGGER_RTT_MAX_NUM_UP_BUFFERS,
    .MaxNumDownBuffers = SEGGER_RTT_MAX_NUM_DOWN_BUFFERS,
    .aUp = {
        {
            .sName = "Terminal",
            .pBuffer = _acUpBuffer,
            .SizeOfBuffer = BUFFER_SIZE_UP,
            .WrOff = 0,
            .RdOff = 0,
            .Flags = 0
        }
    },
    .aDown = {
        {
            .sName = "Terminal",
            .pBuffer = _acDownBuffer,
            .SizeOfBuffer = BUFFER_SIZE_DOWN,
            .WrOff = 0,
            .RdOff = 0,
            .Flags = 0
        }
    }
};

/*********************************************************************
* SEGGER_RTT_Init
*/
void SEGGER_RTT_Init(void) {
    /* Already initialized via static initialization */
    /* Force the control block ID to be correct */
    _SEGGER_RTT.acID[0] = 'S';
    _SEGGER_RTT.acID[1] = 'E';
    _SEGGER_RTT.acID[2] = 'G';
    _SEGGER_RTT.acID[3] = 'G';
    _SEGGER_RTT.acID[4] = 'E';
    _SEGGER_RTT.acID[5] = 'R';
    _SEGGER_RTT.acID[6] = ' ';
    _SEGGER_RTT.acID[7] = 'R';
    _SEGGER_RTT.acID[8] = 'T';
    _SEGGER_RTT.acID[9] = 'T';
    _SEGGER_RTT.acID[10] = '\0';
}

/*********************************************************************
* SEGGER_RTT_Write
*/
unsigned SEGGER_RTT_Write(unsigned BufferIndex, const void* pBuffer, unsigned NumBytes) {
    unsigned WrOff;
    unsigned RdOff;
    unsigned Free;
    unsigned NumBytesToWrite;
    unsigned NumBytesWritten;
    const char* pData;
    SEGGER_RTT_BUFFER_UP* pRing;
    
    if (BufferIndex >= SEGGER_RTT_MAX_NUM_UP_BUFFERS) {
        return 0;
    }
    
    pRing = &_SEGGER_RTT.aUp[BufferIndex];
    pData = (const char*)pBuffer;
    NumBytesWritten = 0;
    
    WrOff = pRing->WrOff;
    RdOff = pRing->RdOff;
    
    /* Calculate free space */
    if (RdOff <= WrOff) {
        Free = pRing->SizeOfBuffer - WrOff + RdOff - 1;
    } else {
        Free = RdOff - WrOff - 1;
    }
    
    NumBytesToWrite = (NumBytes < Free) ? NumBytes : Free;
    
    /* Write data to ring buffer */
    while (NumBytesToWrite > 0) {
        unsigned Chunk;
        
        if (WrOff >= RdOff) {
            Chunk = pRing->SizeOfBuffer - WrOff;
            if (RdOff == 0) {
                Chunk--;
            }
        } else {
            Chunk = RdOff - WrOff - 1;
        }
        
        if (Chunk > NumBytesToWrite) {
            Chunk = NumBytesToWrite;
        }
        
        memcpy(pRing->pBuffer + WrOff, pData, Chunk);
        
        WrOff += Chunk;
        if (WrOff >= pRing->SizeOfBuffer) {
            WrOff = 0;
        }
        
        pData += Chunk;
        NumBytesWritten += Chunk;
        NumBytesToWrite -= Chunk;
    }
    
    pRing->WrOff = WrOff;
    
    return NumBytesWritten;
}

/*********************************************************************
* SEGGER_RTT_WriteString
*/
unsigned SEGGER_RTT_WriteString(unsigned BufferIndex, const char* s) {
    unsigned Len = 0;
    const char* p = s;
    
    while (*p) {
        Len++;
        p++;
    }
    
    return SEGGER_RTT_Write(BufferIndex, s, Len);
}

/*********************************************************************
* SEGGER_RTT_printf - Simple printf implementation
*/
int SEGGER_RTT_printf(unsigned BufferIndex, const char* sFormat, ...) {
    char buf[256];
    va_list args;
    int len = 0;
    const char* p = sFormat;
    char* out = buf;
    
    va_start(args, sFormat);
    
    while (*p && (out - buf) < 250) {
        if (*p == '%') {
            p++;
            switch (*p) {
                case 'd':
                case 'i': {
                    int val = va_arg(args, int);
                    char tmp[12];
                    int i = 0;
                    int negative = 0;
                    
                    if (val < 0) {
                        negative = 1;
                        val = -val;
                    }
                    
                    if (val == 0) {
                        tmp[i++] = '0';
                    } else {
                        while (val > 0) {
                            tmp[i++] = '0' + (val % 10);
                            val /= 10;
                        }
                    }
                    
                    if (negative) {
                        *out++ = '-';
                    }
                    
                    while (i > 0) {
                        *out++ = tmp[--i];
                    }
                    break;
                }
                case 'u': {
                    unsigned val = va_arg(args, unsigned);
                    char tmp[12];
                    int i = 0;
                    
                    if (val == 0) {
                        tmp[i++] = '0';
                    } else {
                        while (val > 0) {
                            tmp[i++] = '0' + (val % 10);
                            val /= 10;
                        }
                    }
                    
                    while (i > 0) {
                        *out++ = tmp[--i];
                    }
                    break;
                }
                case 'x':
                case 'X': {
                    unsigned val = va_arg(args, unsigned);
                    const char* hex = (*p == 'x') ? "0123456789abcdef" : "0123456789ABCDEF";
                    char tmp[8];
                    int i = 0;
                    
                    if (val == 0) {
                        tmp[i++] = '0';
                    } else {
                        while (val > 0) {
                            tmp[i++] = hex[val & 0xF];
                            val >>= 4;
                        }
                    }
                    
                    while (i > 0) {
                        *out++ = tmp[--i];
                    }
                    break;
                }
                case 's': {
                    const char* s = va_arg(args, const char*);
                    while (*s && (out - buf) < 250) {
                        *out++ = *s++;
                    }
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    *out++ = c;
                    break;
                }
                case '%':
                    *out++ = '%';
                    break;
                default:
                    *out++ = '%';
                    *out++ = *p;
                    break;
            }
            p++;
        } else {
            *out++ = *p++;
        }
    }
    
    va_end(args);
    
    len = out - buf;
    SEGGER_RTT_Write(BufferIndex, buf, len);
    
    return len;
}

/*********************************************************************
* SEGGER_RTT_HasKey - Check if input is available
*/
int SEGGER_RTT_HasKey(void) {
    SEGGER_RTT_BUFFER_DOWN* pRing = &_SEGGER_RTT.aDown[0];
    return (pRing->WrOff != pRing->RdOff);
}

/*********************************************************************
* SEGGER_RTT_GetKey - Get a character from input
*/
int SEGGER_RTT_GetKey(void) {
    SEGGER_RTT_BUFFER_DOWN* pRing = &_SEGGER_RTT.aDown[0];
    int r = -1;
    
    if (pRing->WrOff != pRing->RdOff) {
        r = pRing->pBuffer[pRing->RdOff];
        pRing->RdOff++;
        if (pRing->RdOff >= pRing->SizeOfBuffer) {
            pRing->RdOff = 0;
        }
    }
    
    return r;
}
