//based on https://github.com/maximecaron/kamikaze/blob/master/src/main/java/com/kamikaze/docidset/impl/PForDeltaDocIdSet.java
//keep for each inverted list two separate arrays containing
//the last docID and size of each block in words in uncompressed form.

#ifndef COMPRESSED_SET_H__
#define COMPRESSED_SET_H__

#define  DEFAULT_BATCH_SIZE  256
#include "Codec.h"
class CompressedSet {
	unsigned int blocksize_;
	unsigned sizeOfCurrentNoCompBlock; // the number of uncompressed elements that is hold in the currentNoCompBlock
	int* currentNoCompBlock;  // the memory used to store the uncompressed elements. Once the block is full, all its elements are compressed into sequencOfCompBlock and the block is cleared.
	Codec codec; // varint encoding codec    
	unsigned int totalDocIdNum; // the total number of elemnts that have been inserted/accessed so far	
	unsigned int lastAdded; // recently inserted/accessed element	
	unsigned int compressedByteSize;

	int* baseListForOnlyCompBlocks;
    //DeltaIntSegmentArray sequenceOfCompBlocks
public:
	CompressedSet(){
		blocksize_ = DEFAULT_BATCH_SIZE;
		currentNoCompBlock = new unsigned int[blocksize_];
		lastAdded = 0;
		compressedBitSize = 0;
		sizeOfCurrentNoCompBlock = 0;
		totalDocIdNum = 0;
	}
	
	CompressedSet(int batchSize){
		blocksize_ = batchSize;
		currentNoCompBlock = new unsigned int[blocksize_];
		lastAdded = 0;
		compressedBitSize = 0;
		sizeOfCurrentNoCompBlock = 0;
		totalDocIdNum = 0;
	}

    void addDocs(unsigned int docids[],size_t start,size_t len){
	  if (totalDocIdNum == 0) {
		initSet();
	  }
	  if ((len + sizeOfCurrentNoCompBlock) <= blocksize_) {
		memcpy( currentNoCompBlock[sizeOfCurrentNoCompBlock],docids[start], len*4 );
		sizeOfCurrentNoCompBlock += len;
	  } else {
		 // the first block can be completed so fillup a complet block
		 int copyLen = _blockSize - sizeOfCurrentNoCompBlock;
		 memcpy( currentNoCompBlock[sizeOfCurrentNoCompBlock],docids[start], copyLen*4 );
		 sizeOfCurrentNoCompBlock = blocksize_;
		
		 //Add to the list of last element of each block
		 //baseListForOnlyCompBlocks.add(currentNoCompBlock[_blockSize-1]);
		
		 //CompResult compRes = PForDeltaCompressCurrentBlock();
		 //compressedByteSize += compRes.getCompressedSize();      
         //sequenceOfCompBlocks.add(compRes.getCompressedBlock());

         // the middle blocks (copy all possible full block)
         int leftLen = len - copyLen;
         int newStart = start + copyLen;
         while(leftLen > blockSize_) {
	        //baseListForOnlyCompBlocks.add(docids[newStart+_blockSize-1]);
        	memcpy( currentNoCompBlock[0],docids[newStart], blocksize_*4 );
            //compRes = PForDeltaCompressCurrentBlock();
	        //compressedByteSize += compRes.getCompressedSize();      
	        //sequenceOfCompBlocks.add(compRes.getCompressedBlock());
	        leftLen -= blockSize_;
	        newStart += blockSize_;
         }

         // the last block
         if(leftLen > 0) {
            memcpy( currentNoCompBlock[0],docids[newStart], leftLen*4 );
         }
         sizeOfCurrentNoCompBlock = leftLen;
	  }
	  lastAdded = docids[start+len-1];
      totalDocIdNum += len;
    }

    void initSet(){
		memset(currentNoCompBlock,0,blockSize_);
    }

  /**
   *  Flush the data left in the currentNoCompBlock into the compressed data
   * 
   */
  public void flush(int docId)
  {
    //CompResult compRes = PForDeltaCompressCurrentBlock();
    //compressedBitSize += compRes.getCompressedSize();      
    //sequenceOfCompBlocks.add(compRes.getCompressedBlock());
  }

  /**
   * Prefix Sum
   * 
   */
  void preProcessBlock(unsigned int block[], size_t size)
  {
    for(int i=size-1; i>0; --i)
    {
      block[i] = block[i] - block[i-1] - 1; 
    }
  }

  /**
   * Prefix Sum
   * 
   */
  void preProcessBlockOpt(unsigned int block[], size_t start, size_t len)
  {
    for(int i=start+len-1; i>start; --i)
    {
      block[i] = block[i] - block[i-1] - 1; 
    }
  }

  /**
   * Reverse Prefix Sum
   */
  void postProcessBlock(unsigned int block[], size_t size)
  {
    for(int i=1; i<size; ++i)
    {
      block[i] = block[i] + block[i-1] + 1;     
    }
  }

  CompResult PForDeltaCompressCurrentBlock(){ 
    preProcessBlock(currentNoCompBlock, sizeOfCurrentNoCompBlock);
    // return the compressed data 
    //CompResult finalRes = PForDeltaCompressOneBlock(currentNoCompBlock);
    return finalRes;  
  }
	
	//PForDeltaDocIdSet
	~CompressedSet(){
		delete[] currentNoCompBlock;
	}
}

#endif  // COMPRESSED_SET_H__

// DYNAMIC CACHING STRATEGY

// maintains a queue of all pages ordered by their recency information that are in
// main memory and pages that are not in main memory but were evicted recently.
// On a page fault, the system updates statistics on whether the access would have been a hit if 10%, 23%, 37%,
// or 50% of the memory was holding compressed pages
// Periodically, the compressed region
// size is changed to match the compressed region size that improves performance the most.

//compare at run-time an application’s performance on the
//compressed-memory system with an estimation of its performance without compression