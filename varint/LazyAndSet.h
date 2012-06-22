#ifndef LAZY_AND_SET_H__
#define LAZY_AND_SET_H__
#include <vector>
#include "CompressedSet.h"
class LazyAndSet;

class LazyAndSetIterator {
private:
	int lastReturn; 
	vector<CompressedSet::Iterator> iterators;
	LazyAndSet& set;
public:
	LazyAndSetIterator(LazyAndSet* parent);
	int docID();
	int nextDoc();
	int advance(int target);
};

class LazyAndSet {
 public:
	vector<shared_ptr<CompressedSet> > sets_;
	int nonNullSize;

	LazyAndSet(){
		sets_ = vector<shared_ptr<CompressedSet> >();
		nonNullSize = 0;
	}
	LazyAndSet(vector<shared_ptr<CompressedSet> >& sets){
		sets_ = sets;
        nonNullSize = sets.size();
	}
	
	LazyAndSetIterator iterator() {
		LazyAndSetIterator it(this);
		return it;
	}
};

LazyAndSetIterator::LazyAndSetIterator(LazyAndSet* parent) : set(*parent){
	lastReturn = -1;
	if (set.nonNullSize < 1)
	    throw string("Minimum one iterator required");
	
	for (vector<shared_ptr<CompressedSet> >::iterator it = set.sets_.begin(); it!=set.sets_.end(); it++){
		shared_ptr<CompressedSet> set  = *it;
		CompressedSet::Iterator dcit = set->iterator();
		iterators.push_back(dcit);
	}
	lastReturn = (iterators.size() > 0 ? -1 : NO_MORE_DOCS);
}

int LazyAndSetIterator::docID() {
    return lastReturn;
}

int LazyAndSetIterator::nextDoc() {
    // DAAT
    if (lastReturn == NO_MORE_DOCS) 
       return NO_MORE_DOCS;    
    
    CompressedSet::Iterator* dcit = &iterators[0];
    int target = dcit->nextDoc();
    int size = iterators.size();
    int skip = 0;
    int i = 1;
    
    // shortcut: if it reaches the end of the shortest list, do not scan other lists
    if(target == NO_MORE_DOCS) { 
        return (lastReturn = target);
    }
   
    // i is ith iterator
    while (i < size) {
      if (i != skip) {
        dcit = &iterators[i];
        int docId = dcit->Advance(target);
       
        // once we reach the end of one of the blocks, we return NO_MORE_DOCS
        if(docId == NO_MORE_DOCS) {
          return (lastReturn = docId);
        }
    
        if (docId > target) { //  cannot find the target in the next list
          target = docId;
          if(i != 0) {
            skip = i;
            i = 0;
            continue;
          } else { // for the first list, it must succeed as long as the docId is not NO_MORE_DOCS
            skip = 0;
          }
        }

      }
      i++;
    }
   
    return (lastReturn = target);
}

int LazyAndSetIterator::advance(int target) {
//     if (lastReturn == DocIdSetIterator.NO_MORE_DOCS) 
//        return DocIdSetIterator.NO_MORE_DOCS;
//     
//     DocIdSetIterator dcit = iterators[0];
//     target = dcit.advance(target);
//     if(target == DocIdSetIterator.NO_MORE_DOCS) { 
//       return (lastReturn = target);
//     }
//     
//     int size = iterators.length;
//     int skip = 0;
//     int i = 1;
//     while (i < size) {
//       if (i != skip) {
//         dcit = iterators[i];
//         int docId = dcit.advance(target);
//         if(docId == DocIdSetIterator.NO_MORE_DOCS) {
//           return (lastReturn = docId);
//         }
//         if (docId > target) {
//           target = docId;
//           if(i != 0) {
//             skip = i;
//             i = 0;
//             continue;
//           }
//           else
//             skip = 0;
//         }
//       }
//       i++;
//     }
//     return (lastReturn = target);
}
#endif  // LAZY_AND_SET_H__
