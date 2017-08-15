#ifndef BUFFER_H
#define BUFFER_H

#include <stdio.h>
#include <stdlib.h>

#include "typedef.h"

#define BUFFER_MAX_SIZE 8192

typedef struct {
ui8		*buffer;//[BUFFER_MAX_SIZE];
ui32	BPIndex; //buffer bit pointer. point to the current bit
ui32	ByteStart;//byte offset from the start of buffer
ui32	BitLeft;//bit left at the current byte
ui32	ByteInxStart;
ui64	TotalBitsLeft;
ui64	TotalBitsRead;
} stBufBitGet;

//! buffer init
stBufBitGet * BufferInit(ui8 * buf_start,ui8* buf_end,int buf_size);

//! test the bit is byte aligned
int ByteAligned();

//! read N bits to last N bits of return value from buffer without buffer pointer moved
unsigned int NextBits(stBufBitGet * tmpBuf,int n);


//! read N bit from buffer
unsigned int ReadBits(stBufBitGet * tmpBuf,int n);

#endif