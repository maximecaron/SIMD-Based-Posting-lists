// from  http://www.stepanovpapers.com/CIKM_2011.pdf 
// compile with g++  -O3 -mssse3 main.cpp  -o result.bin
using namespace std;

#include <iostream>
#include <stdio.h>
#include <assert.h>
#include "varint/CompressedSet.h"
#include <time.h>
#include "varint/LazyAndSet.h"
#include <vector>
double diffclock(clock_t clock1,clock_t clock2)
{
	double diffticks=clock1-clock2;
	double diffms=(diffticks*1000)/CLOCKS_PER_SEC;
	return diffms;
}

void benchmark(){
		CompressedSet myset1;
		for (unsigned int i = 0; i<=261000000; ++i){
		  myset1.addDoc(i);	
		}
		
		clock_t begin=clock();
		
		// Sequential scanning 261 000 000 docs per second
		for (CompressedSet::Iterator it = myset1.iterator(); it.docID() != NO_MORE_DOCS;it.nextDoc()){
			unsigned int temp = it.docID();
		}

		clock_t end=clock();
        std::cout << "Iteration Time: " << double(diffclock(end,begin)) << " ms"<< endl;
}



int main() {
	benchmark();
}

