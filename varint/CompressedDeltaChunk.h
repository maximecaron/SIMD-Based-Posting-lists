#ifndef  COMPRESSED_DELTA_CHUNK_H__
#define  COMPRESSED_DELTA_CHUNK_H__
#include <vector>
#include "Common.h"
#include "Sink.h"
#include "Source.h"
class CompressedDeltaChunk {
private:
    size_t compressedSize_;
    vector<uint8> data_;
    
    //disable copy constructor
    CompressedDeltaChunk(const CompressedDeltaChunk& other);
    CompressedDeltaChunk& operator=(const CompressedDeltaChunk& other);
public: 
    CompressedDeltaChunk(){
        compressedSize_ = 0; 
    }

    CompressedDeltaChunk(size_t compressedSize):data_(compressedSize){
        compressedSize_ = compressedSize;
    }
    
    CompressedDeltaChunk(istream & in) :compressedSize_(0) {
        in.read((char*)&compressedSize_,4);
		data_.resize(compressedSize_);
        in.read((char*)&(data_[0]),compressedSize_);
        
    }

    void shrinkContainer(vector<uint8> &container) {
      if (container.size() != container.capacity()) {
        vector<uint8> tmp = container;
        swap(container, tmp);
      }
    }

    void resize(size_t newsize){
		data_.resize(newsize);
		compressedSize_ = newsize;
		shrinkContainer(data_);
    }
    vector<uint8>& getVector(){
		return data_;
    }

    ~CompressedDeltaChunk(){
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
        out.write((char*)&(data_[0]),compressedSize_);
    }

}__attribute__ ((aligned (256)));
#endif // COMPRESSED_DELTA_CHUNK_H__