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

  const CompressedDeltaChunk& get(int index) const  {
	return data2.find(index)->second;
  }

  size_t size() const {
    return	currentIndex+1;
  }

};
#endif // DELTA_CHUNK_STORE_H__