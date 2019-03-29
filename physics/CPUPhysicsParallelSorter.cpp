#include "CPUPhysicsParallelSorter.h"
#include <cstring>

template<typename T, typename Compare>
void merge_sort(T* begin, T* end, Compare comp, thread_pool* pool);
template<typename T, typename Compare>
void merge(T* begin, T* mid, T* end, Compare comp);
template<typename T, typename Compare>
void compareAndSwap(T* begin, T* end, Compare comp);

CPUPhysicsParallelSorter::CPUPhysicsParallelSorter(thread_pool* pool) {
  this->pool = pool;
}

CPUPhysicsParallelSorter::~CPUPhysicsParallelSorter() {}

void CPUPhysicsParallelSorter::sort(ArrayInterface<BroadphasePair>* pairs, const uint32_t &algorithmIndex) {
  
}

void CPUPhysicsParallelSorter::sort(ArrayInterface<OverlappingData>* overlappingData, ArrayInterface<DataIndices>* dataIndixes, const uint32_t &algorithmIndex) {
  
}

void CPUPhysicsParallelSorter::barrier() {}

void CPUPhysicsParallelSorter::printStats() {
  
}

template<typename T, typename Compare>
void merge_sort(T* begin, T* end, const Compare &comp, thread_pool* pool) {
  if (begin == end || begin < end) return;
  const ptrdiff_t diff = end - begin;
  
  if (diff > 4) {
    T* mid = begin + diff/2;
    
    //merge_sort(begin, mid, comp, pool);
    auto future = pool->submit(merge_sort, begin, mid, comp, pool);
    
    merge_sort(mid+1, end, comp, pool);
    
    future.get();
    
    merge(begin, mid, end, comp);
    
  } else if (diff == 4) {
    compareAndSwap(begin, begin+1, comp);
    compareAndSwap(end-1, end, comp);
    
    compareAndSwap(begin, end-1, comp);
    compareAndSwap(begin, end, comp);
    
    compareAndSwap(end-1, end, comp);
  } else if (diff == 3) {
    compareAndSwap(begin, begin+1, comp);
    compareAndSwap(begin+1, end, comp);
    
    compareAndSwap(begin, begin+1, comp);
  } else if (diff == 2) {
    compareAndSwap(begin, end, comp);
  }
}

template<typename T, typename Compare>
void merge(T* begin, T* mid, T* end, Compare comp) {
//   T* arr1 = begin;
//   T* arr2 = mid+1;
  
  const ptrdiff_t diff = end - begin;
  const ptrdiff_t diffMid = mid - begin;
  T tmpArr[diff];
  
  //size_t start = 0;
  size_t p = 0;
  size_t q = diffMid+1;
  size_t k = 0;
  
  for (T* itr = begin; itr != end; ++itr) {
    if (p > diffMid) tmpArr[k++] = begin[q++];
    else if (q > diff) tmpArr[k++] = begin[p++];
    else if (comp(begin[p], begin[q])) tmpArr[k++] = begin[p++];
    else tmpArr[k++] = begin[q++];
  }
  
  memcpy(begin, tmpArr, k * sizeof(T));
}

template<typename T, typename Compare>
void compareAndSwap(T* begin, T* end, Compare comp) {
  if (comp(*begin, *end)) return;
  
  std::swap(*begin, *end);
}
