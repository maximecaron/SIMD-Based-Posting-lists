#ifndef  DELTA_CHUNK_STORE_H__
#define  DELTA_CHUNK_STORE_H__
#include <vector>
#include <iostream>
#include <tr1/unordered_map>
#include <tr1/memory>
#include "CompressedDeltaChunk.h"
using namespace std::tr1;
class DeltaChunkStore {
  vector<shared_ptr<CompressedDeltaChunk> > data2;
public:  
  DeltaChunkStore(){
  }

  shared_ptr<CompressedDeltaChunk> allocateBlock(size_t compressedSize){
	shared_ptr<CompressedDeltaChunk> compblock(new CompressedDeltaChunk(compressedSize));
	return compblock;
  }

  void add(shared_ptr<CompressedDeltaChunk> val) {
	data2.push_back(val);
  }

  const CompressedDeltaChunk& get(int index) const  {
	return *data2[index];
  }

  void compact(){
	data2.resize(data2.size());
  }

  size_t size() const {
    return	data2.size();
  }

};
#endif // DELTA_CHUNK_STORE_H__