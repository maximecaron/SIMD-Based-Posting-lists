// from  http://www.stepanovpapers.com/CIKM_2011.pdf 
// compile with g++ -O2 -mssse3 main.cpp  -o result.bin
#include <iostream>
#include <stdio.h>
#include <assert.h>
#include <xmmintrin.h>
#include "varint/Sink.h"
#include "varint/Source.h"
#include "varint/Codec.h"

using namespace std;
int main()
{
  Codec codec;
  unsigned int encodeSrcBuffer[] = {0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8}; 
  int DstBufferSize  = codec.compressed_length(encodeSrcBuffer,8);
  char* encodeDstBuffer = new char[DstBufferSize];
  int numEnmcodedInt = codec.Compress(encodeSrcBuffer,8,encodeDstBuffer,DstBufferSize);
  printf(" encoded  size %i bytes\n",numEnmcodedInt);

  int uncompressedSize  = codec.uncompressed_length(encodeDstBuffer,DstBufferSize);
  unsigned int* decodeDstBuffer = new unsigned int[uncompressedSize];
  int numDecodedInt = codec.Uncompress(encodeDstBuffer,DstBufferSize,decodeDstBuffer,uncompressedSize);
  printf("uncompressedSize = %i bytes,  num decoded  = %i\n",uncompressedSize*4,numDecodedInt);

}
