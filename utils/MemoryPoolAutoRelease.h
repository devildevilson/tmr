/**
 * Реализация MemoryPoolAutoRelease. Выделяется дополнительно 8 байт на блок
 * Для корректного удаления элементов, необходимо вызвать deleteElement
 * для каждого указателя из этого пула!
 * Это версия с авто удалением всех объектов, скорее всего работает крайне не торопливо
 * но с другой стороны, долгое разрушение объектов происходит только в деструкторе
 * и если не создавать этот пул в цикле, то поди и нормально
 */

#ifndef MEMORY_POOL_AUTO_RELEASE_H
#define MEMORY_POOL_AUTO_RELEASE_H

#include <cstdint>
#include <cstddef>
#include <utility>

#include <unordered_set>

template<typename T, size_t blockSize = 4096>
class MemoryPoolAutoRelease {
public:
    template <typename U> struct rebind {
      typedef MemoryPoolAutoRelease<U> other;
    };

    MemoryPoolAutoRelease() noexcept {}
    MemoryPoolAutoRelease(const MemoryPoolAutoRelease& MemoryPoolAutoRelease) noexcept : MemoryPoolAutoRelease() {}
    MemoryPoolAutoRelease(MemoryPoolAutoRelease&& MemoryPoolAutoRelease) noexcept {
        currentBlock_ = MemoryPoolAutoRelease.currentBlock_;
        currentSlot_  = MemoryPoolAutoRelease.currentSlot_;
        lastSlot_     = MemoryPoolAutoRelease.lastSlot_;
        freeSlots_    = MemoryPoolAutoRelease.freeSlots;
        MemoryPoolAutoRelease.currentBlock_ = nullptr;
        MemoryPoolAutoRelease.currentSlot_  = nullptr;
        MemoryPoolAutoRelease.lastSlot_     = nullptr;
        MemoryPoolAutoRelease.freeSlots_    = nullptr;
    }

    template <class U> MemoryPoolAutoRelease(const MemoryPoolAutoRelease<U>& MemoryPoolAutoRelease) noexcept : MemoryPoolAutoRelease() {}

    ~MemoryPoolAutoRelease() noexcept {
      clear();
    }

    MemoryPoolAutoRelease& operator=(const MemoryPoolAutoRelease& MemoryPoolAutoRelease) = delete;

    MemoryPoolAutoRelease& operator=(MemoryPoolAutoRelease&& MemoryPoolAutoRelease) noexcept {
      if (this != &MemoryPoolAutoRelease) {
        std::swap(currentBlock_, MemoryPoolAutoRelease.currentBlock_);
        currentSlot_ = MemoryPoolAutoRelease.currentSlot_;
        lastSlot_ = MemoryPoolAutoRelease.lastSlot_;
        freeSlots_ = MemoryPoolAutoRelease.freeSlots_;
      }

      return *this;
    }

    T* address(T& x) const noexcept {
        return &x;
    }

    const T* address(const T& x) const noexcept {
        return &x;
    }

    T* allocate(const size_t &n = 1, const T* hint = nullptr) {
      (void)n;
      (void)hint;
      if (freeSlots_ != nullptr) {
        T* result = reinterpret_cast<T*>(freeSlots_);
        freeSlots_ = freeSlots_->next;

        return result;
      } else {
        if (currentSlot_ >= lastSlot_) allocateBlock();

        return reinterpret_cast<T*>(currentSlot_++);
      }
    }

    void deallocate(T* p, const size_t &n = 1) {
      (void)n;
      if (p != nullptr) {
        reinterpret_cast<Slot_*>(p)->next = freeSlots_;
        freeSlots_ = reinterpret_cast<Slot_*>(p);
      }
    }

    size_t max_size() const noexcept {
      size_t maxBlocks = -1 / blockSize;
      return (blockSize - sizeof(char*)) / sizeof(Slot_) * maxBlocks;
    }

    template <class U, class... Args> void construct(U* p, Args&&... args) {
      new (p) U (std::forward<Args>(args)...);
    }

    template <class U> void destroy(U* p) {
        p->~U();
    }

    template <class... Args> T* newElement(Args&&... args)  {
      T* result = allocate();
      construct<T>(result, std::forward<Args>(args)...);
      return result;
    }

    void deleteElement(T* p) {
      if (p != nullptr) {
        p->~T();
        deallocate(p);
      }
    }
    
    size_t getSize() {
      return blockSize;
    }

    void clear() {
      std::unordered_set<void*> set;
      Slot_* currFree = freeSlots_;
      while (currFree != nullptr) {
        Slot_* prev = currFree->next;
        set.insert(currFree);
        currFree = prev;
      }
      
      const size_t countInBlock = blockSize / sizeof(T);
      
      Slot_* currDel = currentBlock_;
      while (currDel != nullptr) {
        Slot_* prev = currDel->next;
        
        Slot_* arr = reinterpret_cast<Slot_*>(reinterpret_cast<char*>(currDel) + sizeof(char*));
        arr = reinterpret_cast<Slot_*>(reinterpret_cast<char*>(arr) + padPointer(reinterpret_cast<char*>(arr), alignof(Slot_)));
        for (size_t i = 0; i < countInBlock; ++i) {
          auto itr = set.find(&arr[i]);
          if (itr != set.end()) continue;
          
          arr[i].element.~T();
        }
        
        delete [] reinterpret_cast<char*>(currDel);
        
        currDel = prev;
      }
    }
private:
    union Slot_ {
      T element;
      Slot_* next;
    };

    Slot_* currentBlock_ = nullptr;
    Slot_* currentSlot_  = nullptr;
    Slot_* lastSlot_     = nullptr;
    Slot_* freeSlots_    = nullptr;

    size_t padPointer(char* p, const size_t &align) const noexcept {
      uintptr_t result = reinterpret_cast<uintptr_t>(p);
      return ((align - result) % align);
    }

    void allocateBlock() {
      // выделим память для нового блока + 8 байт под указатель на предыдущий
      size_t size = blockSize + sizeof(Slot_*);
      char* newBlock = new char[size];
      reinterpret_cast<Slot_*>(newBlock)->next = currentBlock_;
      currentBlock_ = reinterpret_cast<Slot_*>(newBlock);

      // немножко пододвинем указатель для того чтобы добиться выравнивания элементов
      char* body = newBlock + sizeof(Slot_*); // в первых 8 байтах у нас лежит указатель
      size_t bodyPadding = padPointer(body, alignof(Slot_)); // получаем выравнивание
      currentSlot_ = reinterpret_cast<Slot_*>(body + bodyPadding); // двигаем указатель
      lastSlot_ = reinterpret_cast<Slot_*>(newBlock + size - sizeof(Slot_) + 1); // находим последний слот
    }
};

#endif
