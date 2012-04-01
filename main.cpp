// from  http://www.stepanovpapers.com/CIKM_2011.pdf 
// compile with g++ -O2 -mssse3 main.cpp  -o result.bin
#include <iostream>
#include <stdio.h>
#include <assert.h>
#include <xmmintrin.h>
typedef uint8_t v16qi __attribute__ ((vector_size (16)));


using namespace std;
char*  mask[256];
int  maskOutputSize[256];

template<typename T>
class Range{
	 size_t capacity;
	 T* begin;
	 T* end;
 public:
	
	template<size_t size>
	Range(T (&a)[size]){
		begin = &a[0];
		end = &a[0] + size;
	}
	
	Range(T a[],size_t size){
		begin = &a[0];
		end = &a[0] + size;
	}
	
	Range(const Range<T>& r){
		begin = r.begin;
		end = r.end;
	}
	
    Range<T> slice(int a,int b){
		Range r = Range<T>(*this);
		if (a < 0) {
			r.begin = end + a;
		} else {
			r.begin = begin + a;
		}
		
		if (b<0){
			r.end = end + b;
		} else {
			r.end = begin + b;
		}
		return r;
    }
	
	void popFront(size_t size = 1 ) {
		assert(!empty());
		begin += size;
	}
	
	
	T& front()  {
		assert(!empty());
		return *begin;
	}
	
	void popBack(size_t size = 1 ) {
		assert(!empty());
		end += size;
	}
	
	
	T& back()  {
		assert(!empty());
		return *end;
	}
	
	void append(const T& value){
		*begin = value;
		begin++;
	}
	
	int length() {
		return end - begin;
	}
	
	bool empty()  {
		return begin >= end; 
	}
};

int getNumByteNeeded(unsigned int value){
	if (value > 0x000000FF ){
	  if (value > 0x0000FFFF){
		  if (value > 0x00FFFFFF){
			return  4;
		  } else {
			return 3;
		 }
	  } else {
		return 2;
	  }
	} else {
	   return 1;	
	}	
}

// input: 
//   src = stream of integer
//   dst = stream of char
// return:
//   number of int that have been written
int encodeBlock(Range<unsigned int>& src,Range<char>& dst){
	unsigned char desc = 0xFF;
	unsigned char bitmask = 0x01;
	unsigned int buffer[8];
	int ithSize[8];
	Range<unsigned int> source = buffer;
	int length = 0;
	int numInt = 0;
	// we cant decode more then 4 byte
	while (!src.empty() ){
		unsigned int temp = src.front();
		int byteNeeded =getNumByteNeeded(temp);
		if (length + byteNeeded > 8){
			break; 
		}
		ithSize[numInt] = byteNeeded;
		numInt++;
		length += byteNeeded;
		source.append(temp);
		src.popFront();
	}
	for (int i =0; i < numInt; i++){
		bitmask = bitmask << (ithSize[i]-1);
		desc = desc ^ bitmask;
		bitmask = bitmask << 1;
	}
	dst.append(desc);
	Range<unsigned int> intsource=buffer;
	for (int i =0; i < numInt; i++){
		int size = ithSize[i];
		unsigned int value = intsource.front();
		intsource.popFront();
		for (int j = 0; j<size; j++){
			dst.append(value >> (j*8));	
		}
		bitmask = bitmask << (ithSize[i]-1);
		desc = desc ^ bitmask;
		bitmask = bitmask << 1;
	}
	return numInt;
}

int decodeBlock(Range<char>& src, Range<int>& dst){
	unsigned char desc = src.front();
	int readSize = maskOutputSize[desc];
	src.popFront();
	// read 16 byte of data even if we only need 8
	v16qi data = __builtin_ia32_lddqu(&src.front());
	// load de required mask
	v16qi shf   = __builtin_ia32_lddqu(mask[desc]);
	v16qi result = __builtin_ia32_pshufb128(data,shf);
	__builtin_ia32_storedqu ((char*)&dst.front(), result);
    if ( readSize > 4) {
      v16qi shf2   = __builtin_ia32_lddqu(mask[desc]+16);
	  v16qi result2 = __builtin_ia32_pshufb128(data,shf2);
	  __builtin_ia32_storedqu (((char*)&dst.front()) + (16), result2);	
	}
	// pop 8 input char
	src.popFront(8);
	
	dst.popFront(readSize);
	
	return readSize;
}

// For all possible values of the
// descriptor we build a table of any shuffle sequence 
// that might be needed at decode time.
void  initShuffleSequence(){
  for (int desc =0;desc<=255;desc++ ){
	// allocate new mask for this
    mask[desc] =new char[32]; 
	int bitmask = 0x00000001;
	int bitindex = 0;
	// count number of 0 in the char
	int complete = 0;
	int ithSize[8];
	int lastpos = -1;
	while (bitindex < 8){
		if ((desc & bitmask) == 0){
			ithSize[complete] = bitindex-lastpos;
			lastpos = bitindex;
			complete++;
		}	
		bitindex++;
		bitmask = bitmask << 1;
	}
	maskOutputSize[desc] = complete;
	
	int j =0;
	int k = 0;
	for (int i =0; i < complete; i++){
		for (int n = 0; n<4; n++){
			if (n < ithSize[i]) {
				mask[desc][k]= j;
				j = j + 1;
			} else {
				mask[desc][k]= -1;
			}
			k = k + 1;
		}
	}
	
	//pad the rest with -1
	for (int i = k; i<16; i++){
		mask[desc][i]= -1;
	}
  }	
}


int main()
{
  initShuffleSequence();

  printf("%s\n","encoding");
  unsigned int encodeSrcBuffer[] = {0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8}; 
  Range<unsigned int> encodeSrc = encodeSrcBuffer;
  char encodeDstBuffer[9];
  Range<char> encodeDst = encodeDstBuffer;
  int numEnmcodedInt = encodeBlock(encodeSrc,encodeDst);
  printf("have encoded %i int\n",numEnmcodedInt);



  printf("%s\n","decoding");
  Range<char> decodingSrc  = encodeDstBuffer;
  int  decodeDstBuffer[8];
  Range<int> decodingDst  = decodeDstBuffer;
  int numDecodedInt = decodeBlock(decodingSrc,decodingDst);
  printf("decoded %i int\n",numDecodedInt);

  Range<int> display = decodeDstBuffer;
  while(numDecodedInt-- ){
  	printf("%X\n",display.front());
  	display.popFront();
  }


}