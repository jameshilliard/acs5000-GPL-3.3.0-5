/*
	rot_buf.c: Buffers control
*/

/* Standard library includes */
#include <unistd.h>
#include <time.h>
#include <string.h>

#include "server.h"

#define MIN(x,y) ((x) > (y) ? (y) : (x))

/* Initialize a buffer for operation */
void InitBuffer(BufferType * B)
  {
	/* Set the initial buffer positions */
	B->RdPos = 0;
	B->WrPos = 0;
  }

/* Check if the buffer is empty */
Boolean IsBufferEmpty(BufferType * B)
  {
	return((Boolean) B->RdPos == B->WrPos);
  }

/* Return chars in buffer */
Boolean inline CharsInBuffer(BufferType * B)
  {
	if (B->WrPos >= B->RdPos)
		return (B->WrPos - B->RdPos);
	else
		return (BufferSize - (B->RdPos - B->WrPos));
  }

/* Check if the buffer is full */
Boolean IsBufferFull(BufferType * B, int cushion)
  {
	int free;

	if (B->WrPos >= B->RdPos) {
		free = BufferSize - B->WrPos + B->RdPos - 1;
	} else {
		free = B->RdPos - B->WrPos - 1;
	}
	return((Boolean)(free < cushion));
  }

/*  Return free space considering cushion */
int inline SpaceInBuffer(BufferType * B, int cushion)
  {
	int free;

	if (B->WrPos >= B->RdPos) {
		free = BufferSize - B->WrPos + B->RdPos - 1;
	} else {
		free = B->RdPos - B->WrPos - 1;
	}
	return (free - cushion);
  }

/* get free buffer available with a cushion */
int GetFreeBufferCushion(BufferType * B, int cushion)
  {
	int free;

	if (B->WrPos >= B->RdPos) {
		free = BufferSize - B->WrPos + B->RdPos - 1;
	} else {
		free = B->RdPos - B->WrPos - 1;
	}
	return(free > cushion ? free - cushion : 0);
  }

/* Calculates the free space available in the buffer */
int MaxContiguosFree(BufferType *B)
  {
	if (B->RdPos == B->WrPos) {
		B->RdPos = B->WrPos = 0;
		return(BufferSize/2);
	}

	if (B->RdPos > B->WrPos) {
		return(B->RdPos - B->WrPos - 1);
	}
	if (B->RdPos) {
		return(BufferSize - B->WrPos);
	} else {
		return(BufferSize - B->WrPos - 1);
	}
  }

/* Get a byte from a buffer */
unsigned char GetFromBuffer(BufferType * B)
  {
	unsigned char C = B->Buffer[B->RdPos];
	B->RdPos = (B->RdPos + 1) % BufferSize;
	return(C);
  }

/* Get a block from a buffer */
void inline GetBlockFromBuffer(char *Cp, BufferType * B, int count)
  {
	  int i;

//The only concern is not to read pass the end of B
	  i = MIN(count, BufferSize - B->RdPos);
	  memcpy(Cp, B->Buffer + B->RdPos, i);
	  B->RdPos = (B->RdPos + i) % BufferSize;

	  if (count -= i) {
		  Cp += i;
		  memcpy(Cp, B->Buffer + B->RdPos, count);
		  B->RdPos = (B->RdPos + count) % BufferSize;
	  }
  }

// ??? Does this work ??? mp: Isn't B a circular buffer?
/* Write count bytes to buffer */
void CopyToBuffer(BufferType * B, char * buf, int count)
  {
	memcpy(&B->Buffer[B->WrPos], buf, count);
	B->WrPos = (B->WrPos + count) % BufferSize;
  }

/* Write a char to buffer */
void PutInBuffer(BufferType * B, char C)
  {
	B->Buffer[B->WrPos] = C;
	B->WrPos = (B->WrPos + 1) % BufferSize;
  }

/* Write a block to buffer */
void inline PutBlockInBuffer(BufferType * B, char *Cp, int count)
  {
	  int i;

//The only concern is not to write pass the end of B, since
//we can rely on the fact that data fits the free space in the buffer
//(SpaceInBuffer() was previously called...)
	  i = MIN(count, BufferSize - B->WrPos);
	  memcpy(B->Buffer + B->WrPos, Cp, i);
	  B->WrPos = (B->WrPos + i) % BufferSize;

	  if (count -= i) {
		  Cp += i;
		  i = MIN(count, BufferSize - B->WrPos);
		  memcpy(B->Buffer + B->WrPos, Cp, i);
		  B->WrPos = (B->WrPos + i) % BufferSize;
	  }
  }

// This routines gets the free space on the buffer B
int GetFreeSpace(BufferType * B) {
	return GetFreeBufferCushion(B, 0);
}

// This routine returns how many bytes are on the buffer B
int bytesAvailable(BufferType * B) {
		    return (BufferSize-1) - GetFreeSpace(B);
}
