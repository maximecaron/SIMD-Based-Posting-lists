#ifndef  COMPRESSED_DELTA_CHUNK_H__
#define  COMPRESSED_DELTA_CHUNK_H__
#include <vector>
#include "Common.h"
#include "Sink.h"
#include "Source.h"
class CompressedDeltaChunk {
	size_t compressedSize_;
	vector<uint8> data_;
public:
	CompressedDeltaChunk(){
		compressedSize_ = 0;	
	}
	CompressedDeltaChunk(size_t compressedSize):data_(compressedSize){
		compressedSize_ = compressedSize;	
	}
	CompressedDeltaChunk(vector<uint8> data,size_t compressedSize){
		compressedSize_ = compressedSize;
		data_ = data;	
	}

	vector<uint8> getCompressedBlock(){
		return data_;	
	}
	size_t getCompressedSize(){
		return compressedSize_;
	}
	
	Sink getSink(){
		return Sink((char*)&(data_[0]),compressedSize_);
	}
	
	Source getSource(){
		return Source((char*)&(data_[0]),compressedSize_);
	}
	
};
#endif // COMPRESSED_DELTA_CHUNK_H__