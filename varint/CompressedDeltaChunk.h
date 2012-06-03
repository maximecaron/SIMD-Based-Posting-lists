#ifndef  COMPRESSED_DELTA_CHUNK_H__
#define  COMPRESSED_DELTA_CHUNK_H__
#include <vector>
#include "Common.h"
#include "Sink.h"
#include "Source.h"
class CompressedDeltaChunk {
private:
	size_t compressedSize_;
	uint8* data_;
	
	//disable copy constructor
	CompressedDeltaChunk(const CompressedDeltaChunk& other);
	CompressedDeltaChunk& operator=(const CompressedDeltaChunk& other);
public:	
    CompressedDeltaChunk(){
    	compressedSize_ = 0;
    	data_ = NULL; 	
    }

	CompressedDeltaChunk(size_t compressedSize){
		compressedSize_ = compressedSize;	
		data_ = new uint8[compressedSize_];
		int datap = (unsigned long)data_;
	}
	
	CompressedDeltaChunk(istream & in) :compressedSize_(0) {
		in.read((char*)&compressedSize_,4);
		data_ = new uint8[compressedSize_];
		in.read((char*)data_,compressedSize_);
	}


	~CompressedDeltaChunk(){
		if ( data_ !=NULL){
		  delete[] data_;
		}
	}

	size_t getCompressedSize(){
		return compressedSize_;
	}
	
	Sink getSink(){
		return Sink((char*)&(data_[0]),compressedSize_);
	}
	
    Source getSource() const {
		return Source((char*)&(data_[0]),compressedSize_);
	}
	
	void write(ostream & out) const{
		out.write((char*)&compressedSize_,4);
		out.write((char*)data_,compressedSize_);
	}

}__attribute__ ((aligned (256)));
#endif // COMPRESSED_DELTA_CHUNK_H__