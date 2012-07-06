#ifndef CODEC_H__
#define CODEC_H__
#include "Common.h"
#include "Source.h"
#include "Sink.h"
#include <iostream>
#include <stdio.h>
#include <emmintrin.h>
#include <sys/mman.h>
class Codec{
private:
    int    maskOutputSize[256];
    char  mask[256][32];
    char*  scratch_output;
    
    // For all possible values of the
    // descriptor we build a table of any shuffle sequence 
    // that might be needed at decode time.
    void init(){
      for (int desc =0;desc<=255;desc++ ){
        // allocate new mask for this
        int bitmask = 0x00000001;
        int bitindex = 0;
        // count number of 0 in the char
        int complete = 0;
        int ithSize[8];
        int lastpos = -1;
        while (bitindex < 8){
            if ((desc & bitmask) == 0){
                ithSize[complete] = bitindex-lastpos;
                lastpos = bitindex;
                complete++;
            }   
            bitindex++;
            bitmask = bitmask << 1;
        }
        maskOutputSize[desc] = complete;

        int j =0;
        int k = 0;
        for (int i =0; i < complete; i++){
            for (int n = 0; n<4; n++){
                if (n < ithSize[i]) {
                    mask[desc][k]= j;
                    j = j + 1;
                } else {
                    mask[desc][k]= -1;
                }
                k = k + 1;
            }
        }
      } 
    
    }

    //Optimize this
    int getNumByteNeeded(unsigned int value){
        if (!value) return 1;
        return (4-(__builtin_clz(value)/8));
    }
 public:
    Codec(){
        // Need a scratch buffer for the output, in case the byte sink doesn't
        // have room for us directly.
        scratch_output = new char[32];
        init(); 
    }
    
    Codec(const Codec& other){
        scratch_output = new char[32];
        init(); 
    }
    
    ~Codec(){
        delete[] scratch_output;
    }
    
    size_t block_compressed_length(Source& src){
        int length = 0;
        int numInt = 0;
        int ithSize[8];
        while (src.Available() > 0){
            size_t n;
            const unsigned int*  temp = (unsigned int*)src.Peek(&n);
            int byteNeeded = getNumByteNeeded(*temp);
            if (PREDICT_FALSE(length + byteNeeded > 8)){
                break; 
            }
            ithSize[numInt] = byteNeeded;
            length += byteNeeded;
            src.Skip(4);
            numInt++;
        }
        int written = 0;
        written++;
        for (int i =0; i < numInt; i++){
            int size = ithSize[i];
            for (int j =0; j < size; j++){
                written++;
            }
        }
        return 9;
    }
    
    
    
    // input: 
    //   src = stream of integer
    //   dst = stream of char
    // return:
    //   number of int that have been written
    int encodeBlock(Source& src,Sink& dst){
        unsigned char desc = 0xFF;
        unsigned char bitmask = 0x01;
        unsigned int buffer[8];
        int ithSize[8];
        int length = 0;
        int numInt = 0;
        

        while (src.Available() > 0){
            size_t n;
            const unsigned int* temp = reinterpret_cast<const unsigned int*>(src.Peek(&n));
            int byteNeeded = getNumByteNeeded(*temp);
            
            if (PREDICT_FALSE((length + byteNeeded) > 8)){
                break; 
            }
            
            //flip the correct in bit desc
            bitmask = bitmask << (byteNeeded-1);
            desc = desc ^ bitmask;
            bitmask = bitmask << 1;
            
            ithSize[numInt] = byteNeeded-1;
            length += byteNeeded;
            buffer[numInt] = *temp;
            src.Skip(4);
            numInt++;
        }
        
        char* dest = dst.GetAppendBuffer(9, scratch_output);
        dest[0] = desc;
        int written = 1;
        for (int i =0; i < numInt; i++){
            int size = ithSize[i];
            unsigned int value = buffer[i];
            for (int j = 0; j<=size; j++){
                dest[written] = value;
                value = value >> 8;
                written++;
            }
        }

        dst.Append(dest, 9);
        return 9;
    }
    
    size_t Compress(Source& src, Sink& sink){
       size_t compressed_size = 0;
       while (src.Available() > 0 ){
         size_t temp  = encodeBlock(src,sink);
         compressed_size += temp;
       }
       return compressed_size;
    }
 
    /**
     * return length of compressed output.
     */
    size_t compressed_length(Source& src){
        size_t compressed_size = 0;
        while (src.Available() ){
            size_t temp = block_compressed_length(src);
            compressed_size +=temp;
        }
        return compressed_size; 
    }
    
    template<typename srctype>
    size_t compressed_length(srctype src,size_t srcSize){
        Source encodeSrc(src,srcSize);
        return compressed_length(encodeSrc);    
    }
    
    /**
     * @return the compressed size in bytes
     */
    template<typename srctype, typename dstype>
    size_t Compress(srctype src, size_t srcSize, dstype dst,size_t dstSize){
       Source encodeSrc(src,srcSize);
       Sink encodeDst((char*)dst,sizeof(*dst)*dstSize);
       return Compress(encodeSrc,encodeDst);
    }

    /**
     * src : a compressed buffer
     * dst : a buffer for uncompressed data of size uncompressed_length(src)
     */
    int decodeBlock(Source& src,Sink& dst) const {
	static char buff[16];
        size_t n;
        const char*  pdesc = (char*)src.Peek(&n);
        unsigned char desc = *pdesc;
        src.Skip(1);

        char* peek = (char*)src.Peek(&n);
		v16qi data;        
        
        // load de required mask
        data = __builtin_ia32_lddqu(peek);
        v16qi shf   = __builtin_ia32_lddqu(mask[desc]);
        v16qi result = __builtin_ia32_pshufb128(data,shf);
        char* dest = dst.GetAppendBuffer(32, scratch_output);
        __builtin_ia32_storedqu (dest, result);

        data = __builtin_ia32_lddqu(peek);
        shf   = __builtin_ia32_lddqu(mask[desc]+16);
        result = __builtin_ia32_pshufb128(data,shf);
        __builtin_ia32_storedqu (dest + (16), result);   

        // pop 8 input char
        src.Skip(8);
        int readSize = maskOutputSize[desc];
        dst.Append(dest, readSize*4);
        return readSize;
    }

    __inline__ int Uncompress(Source& src, Sink& sink) const {
       size_t uncompressSize = 0;
       while (src.Available() > 0){
         uncompressSize += decodeBlock(src,sink);
       }
       return uncompressSize;
    }

    __inline__ size_t Uncompress(Source& src, unsigned int* dst,size_t size) const {
       Sink decodeDst((char*)dst,sizeof(*dst)*size);
       return Uncompress(src,decodeDst);
    }

    /**
     * @return the uncompressed size in bytes
     */
    template<typename srctype, typename dstype>
    size_t Uncompress(srctype src,size_t srcSize, dstype dst,size_t dstSize){
       Source decodeSrc(src,srcSize);
       Sink decodeDst((char*)dst,sizeof(*dst)*dstSize);
       return Uncompress(decodeSrc,decodeDst);
    }
    
    /**
     * return length of uncompressed output.
     */
    size_t uncompressed_length(Source src){
        size_t size = 0;
        size_t avail = 0;
        const char*  desc = (char*)src.Peek(&avail);
        while (avail >= 9){
             size_t readSize = maskOutputSize[*desc];
             size +=  readSize;
             desc +=9;
             avail -= 9;
        }
        return size;
    }
    
    template<typename srctype>
    size_t uncompressed_length(srctype src,size_t srcSize){
        Source decodeSrc(src,srcSize);
        return uncompressed_length(decodeSrc);
    }
        
    
    bool findInDeltaArray(unsigned int array[],size_t size,unsigned int target) const {       
       unsigned int idx;
       unsigned int lastId = array[0];
       if (lastId == target) return true;

       // searching while doing prefix sum (to get docIds instead of d-gaps)
       for(idx = 1; idx<size; ++idx){
         lastId += (array[idx]+1);
         if (lastId >= target)
            return (lastId == target);
       }
       return false;
    }
};

#endif  // CODEC_H__