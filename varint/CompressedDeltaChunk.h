#ifndef  COMPRESSED_DELTA_CHUNK_H__
#define  COMPRESSED_DELTA_CHUNK_H__
#include <vector>
#include "Common.h"
#include "Sink.h"
#include "Source.h"
class CompressedDeltaChunk {
	size_t compressedSize_;
	vector<uint8> data_;
//	Source* src;
public:
	CompressedDeltaChunk(){
		compressedSize_ = 0;
	//	src = NULL; 	
	}
	CompressedDeltaChunk(const CompressedDeltaChunk& other){
		compressedSize_ = 0;
	//	src = new Source((char*)&(data_[0]),compressedSize_); 	
	}
	CompressedDeltaChunk(size_t compressedSize):data_(compressedSize){
		compressedSize_ = compressedSize;
	//	src = new Source((char*)&(data_[0]),compressedSize_);
	}
	CompressedDeltaChunk(vector<uint8> data,size_t compressedSize){
		compressedSize_ = compressedSize;
		data_ = data;
	//	src = new Source((char*)&(data_[0]),compressedSize_);
	}
	~CompressedDeltaChunk(){
	//	if ( src !=NULL){
	//	  delete src;
	//	}
	}

	size_t getCompressedSize(){
		return compressedSize_;
	}
	
	Sink getSink(){
		return Sink((char*)&(data_[0]),compressedSize_);
	}
	
    Source getSource(){
		//return *src;
		return Source((char*)&(data_[0]),compressedSize_);
	}
	
};
#endif // COMPRESSED_DELTA_CHUNK_H__