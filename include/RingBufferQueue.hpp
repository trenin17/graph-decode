#include "MemoryPool.hpp"

template <typename T, size_t capacity>
class RingBufferQueue {
private:
    std::array<T, capacity> arr;        
    int front_ = 0, size_ = 0; 

public:

    bool empty() { 
        return size_ == 0; 
    }
  
    // Get the front element
    T& front() { 
        return arr[front_];
    }

    // Insert an element at the rear
    void push(const T& x) {
        // Queue is full
        if (size_ == capacity) 
            return; 
        int rear = (front_ + size_) % capacity;
        arr[rear] = x;
        size_++;
    }

    void push(T&& x) {
        // Queue is full
        if (size_ == capacity) 
            return; 
        int rear = (front_ + size_) % capacity;
        arr[rear] = std::move(x);
        size_++;
    }

    // Remove an element from the front
    void pop() {
        front_ = (front_ + 1) % capacity;
        size_--;
    }
};