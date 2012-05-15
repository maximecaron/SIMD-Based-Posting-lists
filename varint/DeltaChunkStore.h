#ifndef  DELTA_CHUNK_STORE_H__
#define  DELTA_CHUNK_STORE_H__
#include <vector>
#include <iostream>
#include <tr1/unordered_map>

#include "CompressedDeltaChunk.h"
using namespace std::tr1;
class DeltaChunkStore {
  unordered_map<int,CompressedDeltaChunk> data2;
  int currentIndex;
public:  
  DeltaChunkStore(){
	currentIndex = -1;
  }

  void add(CompressedDeltaChunk val) {
	currentIndex++;
	data2[currentIndex] = val;
  }

  void set(CompressedDeltaChunk val,int index) {
	data2[index] = val;
  }

  CompressedDeltaChunk& get(int index) {
	return data2[index];
  }

  size_t size() {
    return	currentIndex+1;
  }

};
#endif // DELTA_CHUNK_STORE_H__