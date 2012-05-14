//based on https://github.com/maximecaron/kamikaze/blob/master/src/main/java/com/kamikaze/docidset/impl/PForDeltaDocIdSet.java
//keep for each inverted list two separate arrays containing
//the last docID and size of each block in words in uncompressed form.

#ifndef COMPRESSED_SET_H__
#define COMPRESSED_SET_H__

#include <iostream>
#include <stdio.h>
#include <limits>

#define  DEFAULT_BATCH_SIZE  256
#include "Codec.h"
#include "DeltaChunkStore.h"
#include "CompressedDeltaChunk.h"
#include "Source.h"
#include "Sink.h"
#include <vector>
const int NO_MORE_DOCS = std::numeric_limits<int>::max();
class CompressedSet;
class SetIterator{
	int BLOCK_INDEX_SHIFT_BITS; // floor(log(blockSize))
	int cursor; // the current pointer of the input 
	// the docId that was accessed of the last time called nextDoc() or advance(),
	// therefore, it is kind of synced with the above three too
	int lastAccessedDocId; 
	int compBlockNum; // the number of compressed blocks
	unsigned int*  iterDecompBlock; // temporary storage for the decompressed data
	Codec codec; // varint encoding codec
	//parent
	CompressedSet& set;	
public:
	SetIterator(CompressedSet& parentSet);
	~SetIterator();
	int docID();
	int getBlockIndex(int docIdIndex);\
	int nextDoc();
};


class CompressedSet {
	unsigned sizeOfCurrentNoCompBlock; // the number of uncompressed elements that is hold in the currentNoCompBlock
	Codec codec; // varint encoding codec    
	unsigned int lastAdded; // recently inserted/accessed element	
	unsigned int compressedByteSize;	
	vector<unsigned int> baseListForOnlyCompBlocks;

public:
	size_t blocksize_;
	unsigned int totalDocIdNum; // the total number of elemnts that have been inserted/accessed so far	
	unsigned int* currentNoCompBlock;  // the memory used to store the uncompressed elements. Once the block is full, all its elements are compressed into sequencOfCompBlock and the block is cleared.
	DeltaChunkStore sequenceOfCompBlocks; // Store for list compressed delta chunk 
	
	
	CompressedSet(int batchSize = DEFAULT_BATCH_SIZE ){
		blocksize_ = batchSize;
		currentNoCompBlock = new unsigned int[blocksize_];
		lastAdded = 0;
		compressedByteSize = 0;
		sizeOfCurrentNoCompBlock = 0;
		totalDocIdNum = 0;
	}
	
	~CompressedSet(){
		delete[] currentNoCompBlock;
    }

    SetIterator iterator() {
       return SetIterator(*this);
    }

    void addDocs(unsigned int docids[],size_t start,size_t len){
	  if (totalDocIdNum == 0) {
		initSet();
	  }
	  if ((len + sizeOfCurrentNoCompBlock) <= blocksize_) {
		memcpy( &currentNoCompBlock[sizeOfCurrentNoCompBlock],&docids[start], len*4 );
		sizeOfCurrentNoCompBlock += len;
	  } else {		
		 // the first block can be completed so fillup a complet block
		 int copyLen = blocksize_ - sizeOfCurrentNoCompBlock;
		 memcpy( &currentNoCompBlock[sizeOfCurrentNoCompBlock],&docids[start], copyLen*4 );
		 sizeOfCurrentNoCompBlock = blocksize_;
		
		 //Add to the list of last element of each block
		 baseListForOnlyCompBlocks.push_back(currentNoCompBlock[blocksize_-1]);
		 CompressedDeltaChunk compRes = PForDeltaCompressCurrentBlock();
		 compressedByteSize += compRes.getCompressedSize();      
         sequenceOfCompBlocks.add(compRes);

         // the middle blocks (copy all possible full block)
         int leftLen = len - copyLen;
         int newStart = start + copyLen;
         while(leftLen > blocksize_) {
	        baseListForOnlyCompBlocks.push_back(docids[newStart+blocksize_-1]);
        	memcpy( &currentNoCompBlock[0],&docids[newStart], blocksize_*4 );

            PForDeltaCompressCurrentBlock();
            compRes = PForDeltaCompressCurrentBlock();
	        compressedByteSize += compRes.getCompressedSize();      
	        sequenceOfCompBlocks.add(compRes);
	        leftLen -= blocksize_;
	        newStart += blocksize_;
         }

         // the last block
         if(leftLen > 0) {
            memcpy( &currentNoCompBlock[0],&docids[newStart], leftLen*4 );
         }
         sizeOfCurrentNoCompBlock = leftLen;
	  }
	  lastAdded = docids[start+len-1];
      totalDocIdNum += len;
    }

  /**
   * Add document to this set
   * 
   */
  void addDoc(unsigned int docId) {
	if(totalDocIdNum==0){
		initSet();
		currentNoCompBlock[sizeOfCurrentNoCompBlock++] = docId;    
		lastAdded = docId;
	} else if (sizeOfCurrentNoCompBlock == blocksize_) {
	    //the last docId of the block      
    	baseListForOnlyCompBlocks.push_back(lastAdded);

	    // compress currentNoCompBlock[] (excluding the input docId),
		CompressedDeltaChunk compRes = PForDeltaCompressCurrentBlock();
		compressedByteSize += compRes.getCompressedSize();      
        sequenceOfCompBlocks.add(compRes);


	    // next block
	    sizeOfCurrentNoCompBlock = 1;
	    lastAdded = docId;
	    currentNoCompBlock[0] = docId;	
	} else {	
	   lastAdded = docId;
	   currentNoCompBlock[sizeOfCurrentNoCompBlock++] = docId;
	}
	totalDocIdNum++;
  }




    void initSet(){
		memset(currentNoCompBlock,0,blocksize_);
    }

  /**
   *  Flush the data left in the currentNoCompBlock into the compressed data
   * 
   */
  void flush()
  {
	baseListForOnlyCompBlocks.push_back(currentNoCompBlock[sizeOfCurrentNoCompBlock-1]);
	preProcessBlock(currentNoCompBlock, sizeOfCurrentNoCompBlock);
	CompressedDeltaChunk compRes = PForDeltaCompressOneBlock(currentNoCompBlock,sizeOfCurrentNoCompBlock);
	
    compressedByteSize += compRes.getCompressedSize();      
    sequenceOfCompBlocks.add(compRes);
	sizeOfCurrentNoCompBlock = 0;

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
  void postProcessBlock(unsigned int block[], size_t size){
    for(int i=1; i<size; ++i) {
      block[i] = block[i] + block[i-1] + 1;     
    }
  }


  CompressedDeltaChunk PForDeltaCompressOneBlock(unsigned int* block,size_t blocksize){
	size_t compressedBlockSize = codec.compressed_length(block,blocksize);
	CompressedDeltaChunk compblock(compressedBlockSize);
	Source src(block,blocksize);
	Sink dst = compblock.getSink();
	size_t compressedSize = codec.Compress(src,dst);
	assert(compressedSize == compressedBlockSize);
	return compblock;
  }

  CompressedDeltaChunk PForDeltaCompressCurrentBlock(){ 
    preProcessBlock(currentNoCompBlock, sizeOfCurrentNoCompBlock);
    CompressedDeltaChunk finalRes = PForDeltaCompressOneBlock(currentNoCompBlock,blocksize_);
    return finalRes;  
  }

  /**
   *  Binary search in the base list for the block that may contain docId greater than or equal to the target 
   * 
   */
  int binarySearchInBaseListForBlockThatMayContainTarget(vector<unsigned int>& in, int start, int end, int target)
  {   
    //the baseListForOnlyCompBlocks (in) contains all last elements of the compressed blocks. 
    return binarySearchForFirstElementEqualOrLargerThanTarget(in, start, end, target);
  }

  /**
   * Binary search for the first element that is equal to or larger than the target 
   * 
   * @param in must be sorted and contains no duplicates
   * @param start
   * @param end
   * @param target
   * @return the index of the first element in the array that is equal or larger than the target. -1 if the target is out of range.
   */
  int binarySearchForFirstElementEqualOrLargerThanTarget(vector<unsigned int>& in, int start, int end, int target) {
    int mid;
    while(start < end) {
      mid = (start + end)/2;
      if(in[mid] < target)
        start = mid+1;
      else if(in[mid] == target)
        return mid;
      else
        end = mid;
    }
    if(in[start] >= target)
      return start;
    else
      return -1;
  }

  /**
   * Linear search for the first element that is equal to or larger than the target 
   */
	int searchForFirstElementEqualOrLargerThanTarget(vector<unsigned int> in, int start, int end, int target) {
      while(start <= end) {
        if(in[start] >= target)
          return start;
        start++;
      }
      return -1;
    }

  /**
   * Regular Binary search for the the target 
   * 
   * @param vals must be sorted
   * @param start
   * @param end
   * @param target
   * @return the index of the target in the input array. -1 if the target is out of range.
   */
	int binarySearchForTarget(vector<unsigned int> vals, int start, int end, int target)
	{
	  int mid;
	  while(start <= end)
	  {
	    mid = (start+end)/2;
	    if(vals[mid]<target)
	      start = mid+1;
	    else if(vals[mid]==target)
	      return mid;
	    else
	      end = mid-1;
	  }
	  return -1;
	}
	
	/**
     * Number of compressed units (for example, docIds) plus the last block
     * @return docset size
     */
    int size() {
      return totalDocIdNum;
    }

    //This method will not work after a call to flush()
	bool find(unsigned int target)
	  { 
		unsigned int myDecompBlock[blocksize_];
	    //unsigned int lastId = lastAdded;
		if(size()==0)
		      return false;
        if (sizeOfCurrentNoCompBlock!=0){
		    int lastId = currentNoCompBlock[sizeOfCurrentNoCompBlock-1];
		    if(target > lastId)
		    {
		      return false;
		    }

		    // first search noComp block
		    if(baseListForOnlyCompBlocks.size()==0 || target>baseListForOnlyCompBlocks[baseListForOnlyCompBlocks.size()-1])
		    {
		      int i;
		      for(i=0; i<sizeOfCurrentNoCompBlock; ++i)
		      {
		        if(currentNoCompBlock[i] >= target)
		          break;
		      }
		      if(i == sizeOfCurrentNoCompBlock) 
		        return false;
		      return currentNoCompBlock[i] == target; 
		    }
		}


		// if we have some CompBlocks
		// first find which block to decompress by looking into baseListForOnlyCompBlocks
        if(baseListForOnlyCompBlocks.size()>0) {
	        // baseListForOnlyCompBlocks.size() must then >0
	       int iterDecompBlock = binarySearchInBaseListForBlockThatMayContainTarget(baseListForOnlyCompBlocks, 0, baseListForOnlyCompBlocks.size()-1, target);
	       if(iterDecompBlock<0)
	         return false; // target is bigger then biggest value
		    
		   ////uncompress block 
		   Source src = sequenceOfCompBlocks.get(iterDecompBlock).getSource();
           size_t uncompSize = codec.Uncompress(src,myDecompBlock,blocksize_);
		   return codec.findInDeltaArray(myDecompBlock,uncompSize,target);
        }
		return false;
	  }
	

	
	//PForDeltaDocIdIterator

};

    SetIterator::SetIterator(CompressedSet& parentSet) : set(parentSet){
        compBlockNum = set.sequenceOfCompBlocks.size();
		cursor = -1;
		int i=-1;
		lastAccessedDocId = -1;
		iterDecompBlock  = new unsigned int[set.blocksize_];
		for(int x=set.blocksize_; x>0; ++i, x = x>> 1) {
			
		}
	    BLOCK_INDEX_SHIFT_BITS = i;
    }

   SetIterator::~SetIterator(){
	   delete[] iterDecompBlock;
   }

    int SetIterator::docID(){
		return lastAccessedDocId;
    }
    /**
     * Get the index of the batch this cursor position falls into
     * 
     * @param index
     * @return
     */
    int SetIterator::getBlockIndex(int docIdIndex) {
      return docIdIndex >> BLOCK_INDEX_SHIFT_BITS;
    }

    int SetIterator::nextDoc(){
	  if(set.totalDocIdNum <= 0 || cursor == set.totalDocIdNum) {
		    //the pointer points past the end
	        lastAccessedDocId = NO_MORE_DOCS;
	        return lastAccessedDocId;
	  }
	  //: if the pointer points to the end
	  if(++cursor == set.totalDocIdNum) { 
        lastAccessedDocId = NO_MORE_DOCS;
        return lastAccessedDocId;
      }
      int iterBlockIndex = getBlockIndex(cursor); // get the block No
	  int offset = cursor % set.blocksize_; // sync offset with cursor
	  //printf("iterBlockIndex %d\n",iterBlockIndex);
	  //printf("offset %d\n",offset);
      //printf("cursor %d\n",cursor);
	  // case 1: in the currentNoCompBlock[] array which has never been compressed yet
	  // and therefore not added into sequenceOfCompBlocks yet.
	  if(iterBlockIndex == compBlockNum) 		  { 
	      lastAccessedDocId = set.currentNoCompBlock[offset];
	      //printf("iterBlockIndex == compBlockNum");
	  } else if (offset == 0){ // must be in one of the compressed blocks
		  Source src = set.sequenceOfCompBlocks.get(iterBlockIndex).getSource();
		  size_t uncompSize = codec.Uncompress(src,iterDecompBlock,set.blocksize_);
		  lastAccessedDocId = iterDecompBlock[offset];
	  } else {
		  lastAccessedDocId += (iterDecompBlock[offset]+1);
	  }
	  return lastAccessedDocId;
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