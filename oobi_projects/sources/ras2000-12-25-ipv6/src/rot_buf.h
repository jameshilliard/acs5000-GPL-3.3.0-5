#ifndef ROT_BUF_H
#define ROT_BUF_H 1

/* Standard boolean definition */
typedef enum { False, True } Boolean;

/* Buffer size */
#define BufferSize 1024

/* Buffer structure */
typedef
  struct
    {
      unsigned char Buffer[BufferSize];
      unsigned int RdPos;
      unsigned int WrPos;
    }
  BufferType;

/* Function prototypes */

/* Initialize a buffer for operation */
void InitBuffer(BufferType * B);

/* Check if the buffer is empty */
Boolean IsBufferEmpty(BufferType * B);

/* Check if the buffer is full */
Boolean IsBufferFull(BufferType * B, int cushion);

/* Calculates the free space available in the buffer */
int MaxContiguosFree(BufferType *B);

/* Get a byte from a buffer */
unsigned char GetFromBuffer(BufferType * B);

/* get free buffer available with a cushion */
int GetFreeBufferCushion(BufferType * B, int cushion);

void PutInBuffer (BufferType * B, char C);

int GetFreeSpace(BufferType * B);

int bytesAvailable(BufferType *B);

void CopyToBuffer(BufferType * B, char * buf, int count);

#endif
