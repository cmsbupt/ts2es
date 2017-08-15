#include "buffer.h"
#include <memory.h>

//! buffer init
stBufBitGet * BufferInit(ui8 * buf_start,ui8* buf_end,int buf_size)
{
	stBufBitGet * tmpBuf= (stBufBitGet * ) malloc( sizeof(stBufBitGet));
	if( tmpBuf == NULL )
		return NULL;
	tmpBuf->BPIndex =0;
	tmpBuf->ByteStart=0;
	tmpBuf->BitLeft=8;
	tmpBuf->ByteInxStart =0;
	tmpBuf->buffer =buf_start;
	tmpBuf->TotalBitsLeft = ((buf_end-buf_start)<<3);
	tmpBuf->TotalBitsRead=0;
	return tmpBuf;
}
//! 
//! test the bit is byte aligned
int ByteAligned(stBufBitGet * tmpBuf)
{
	return tmpBuf->BitLeft==8?1:0;
}

//! read N bits to last N bits of return value from buffer without buffer pointer moved
unsigned int NextBits(stBufBitGet * tmpBuf,int n)
{
	unsigned int tmp=0;
	unsigned int BitNum=0;
	unsigned int sByteStart;
	unsigned int sBitLeft;
	unsigned char TmpByte;
	int i;
	if ( n > 32 || n<=0 )
	{
		return -1;
	} 
	else// 1 <= n <=32
	{
		if (  tmpBuf->TotalBitsLeft < n )
		{
			printf("error from NextBits dddddd:TotalBitsLeft=%I64d less than %d bits  \n",tmpBuf->TotalBitsLeft,n);
			exit(0);
			//return 0;
		}
		sByteStart= tmpBuf->ByteStart;
		sBitLeft= tmpBuf->BitLeft;
		if (tmpBuf->BitLeft!=8)//read the left bits in current byte 
		{
			if(n<tmpBuf->BitLeft) 
			{
				TmpByte=tmpBuf->buffer[tmpBuf->ByteStart];
				TmpByte=TmpByte<<(8-tmpBuf->BitLeft);
				tmp=TmpByte>>(8-n);
				tmpBuf->BitLeft=tmpBuf->BitLeft-n;
				//return tmp;
			}
			else// n >= BitLeft
			{
				TmpByte=tmpBuf->buffer[tmpBuf->ByteStart];
				TmpByte=TmpByte<<(8-tmpBuf->BitLeft);
				tmp=TmpByte>>(8-tmpBuf->BitLeft);
				//BitNum=BitLeft;//read BitNum bits to tmp
				tmpBuf->ByteStart++;
				n=n-tmpBuf->BitLeft;
				tmpBuf->BitLeft=8;
				for (i=0; i<n/8; i++)//read n/8 byte
				{
					TmpByte=tmpBuf->buffer[tmpBuf->ByteStart];
					tmp=tmp<<8;
					tmp=tmp+TmpByte;
					tmpBuf->ByteStart++;
				}
				if(n%8)
				{
					TmpByte=tmpBuf->buffer[tmpBuf->ByteStart];
					tmp=tmp<<(n%8);
					TmpByte=TmpByte>>(8-n%8);
					tmp=tmp+TmpByte;
					tmpBuf->BitLeft=8-n%8;
				}
				//return tmp;
			}
		}
		else//BitLeft=8,byte alignd
		{
			tmp=0;
			TmpByte=0;
			for (i=0; i<n/8; i++)//read n/8 byte
			{
				TmpByte=tmpBuf->buffer[tmpBuf->ByteStart];
				tmp=tmp<<8;
				tmp=tmp+TmpByte;
				tmpBuf->ByteStart++;
			}
			if(n%8)
			{
				TmpByte=tmpBuf->buffer[tmpBuf->ByteStart];
				tmp=tmp<<(n%8);
				TmpByte=TmpByte>>(8-n%8);
				tmp=tmp+TmpByte;
				tmpBuf->BitLeft=8-n%8;
			}
		}
		tmpBuf->ByteStart=sByteStart;
		tmpBuf->BitLeft=sBitLeft;
		return tmp;
	}
}

//! read N bit from buffer
unsigned int ReadBits(stBufBitGet * tmpBuf,int n)
{
	unsigned int tmp=0;
	unsigned char TmpByte=0;
	int i,tmpn=n;
	//printf("ReadBits:TotalBitsLeft=%I64d ,read %d bits  \n",tmpBuf->TotalBitsLeft,n);
	if (  tmpBuf->TotalBitsLeft < n )
	{
		printf("error from ReadBits:TotalBitsLeft=%I64d less than %d bits  \n",tmpBuf->TotalBitsLeft,n);
		//exit(0);
	}
	if ( n > 32 || n<=0 )
	{
		return -1;
	} 
	else// 1 <= n <=32
	{

		if (tmpBuf->BitLeft!=8)//read the left bits in current byte
		{
			if(n<tmpBuf->BitLeft)
			{
				TmpByte=tmpBuf->buffer[tmpBuf->ByteStart];
				TmpByte=TmpByte<<(8-tmpBuf->BitLeft);
				tmp=(TmpByte)>>(8-n);
				tmpBuf->BitLeft=tmpBuf->BitLeft-n;
			}
			else// n >= BitLeft
			{
				TmpByte=tmpBuf->buffer[tmpBuf->ByteStart];
				TmpByte=TmpByte<<(8-tmpBuf->BitLeft);
				tmp=TmpByte>>(8-tmpBuf->BitLeft);
				tmpBuf->ByteStart++;
				n=n-tmpBuf->BitLeft;
				tmpBuf->BitLeft=8;
				for (i=0; i<n/8; i++)//read n/8 byte
				{
					TmpByte=tmpBuf->buffer[tmpBuf->ByteStart];
					tmp=tmp<<8;
					tmp=tmp+TmpByte;
					tmpBuf->ByteStart++;
				}
				if(n%8)
				{
					TmpByte=tmpBuf->buffer[tmpBuf->ByteStart];
					tmp=tmp<<(n%8);
					TmpByte=TmpByte>>(8-n%8);
					tmp=tmp+TmpByte;
					tmpBuf->BitLeft=8-n%8;
				}
			}
		}
		else//BitLeft=8,byte alignd
		{
			for (i=0; i<n/8; i++)//read n/8 byte
			{
				TmpByte=tmpBuf->buffer[tmpBuf->ByteStart];
				tmp=(tmp<<8)+TmpByte;
				tmpBuf->ByteStart= tmpBuf->ByteStart + 1;
			}
			if(n%8)
			{
				TmpByte=tmpBuf->buffer[tmpBuf->ByteStart];
				tmp=(tmp<<(n%8));
				tmp=tmp+(TmpByte>>(8-n%8));
				tmpBuf->BitLeft=8-n%8;
			}
		}
		tmpBuf->TotalBitsLeft = tmpBuf->TotalBitsLeft - tmpn;
		tmpBuf->TotalBitsRead=tmpBuf->TotalBitsRead+tmpn;
		return tmp;
	}
}
//! read N bytes from file to buffer with offset bytes skiped 
