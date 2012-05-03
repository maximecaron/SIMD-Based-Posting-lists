// from  http://www.stepanovpapers.com/CIKM_2011.pdf 
// compile with g++ -O2 -mssse3 main.cpp  -o result.bin
using namespace std;

#include <iostream>
#include <stdio.h>
#include <assert.h>

#include "varint/CompressedSet.h"

int main() {
	unsigned int encodeSrcBuffer[] = {0x00,0xFFF,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xD1,0xD2,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9}; 
	CompressedSet set;
	set.addDocs(encodeSrcBuffer,0,16);
	

	for(SetIterator it  = set.iterator(); it.docID() != NO_MORE_DOCS; it.nextDoc()){
		printf("%X\n",it.docID());	
	}

    printf("Done\n");
}
