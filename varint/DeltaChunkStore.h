#ifndef  DELTA_CHUNK_STORE_H__
#define  DELTA_CHUNK_STORE_H__
#include <vector>
#include "CompressedDeltaChunk.h"
class DeltaChunkStore {
  vector<CompressedDeltaChunk> data;
public: 
  void add(CompressedDeltaChunk val) {
	data.push_back(val);
  }

  void set(CompressedDeltaChunk val,int index) {
	data.resize(index+1);
	data[index] = val;
  }

  CompressedDeltaChunk get(int index) {
	return data[index];
  }

  size_t size() {
	return data.size();
  }

};
#endif // DELTA_CHUNK_STORE_H__