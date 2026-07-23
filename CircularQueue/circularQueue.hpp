#pragma once
#include<array>
#include<condition_variable>
#include<mutex>
#include<optional>
#include<atomic>

template<typename T, size_t Capacity>
class CircularQueue{

    static_assert((Capacity & (Capacity-1))==0,"Capacity must be poewr of two");

    std::array<T, Capacity> buffer;
    std::atomic<size_t> head = 0;
    std::atomic<size_t> tail = 0;

    static constexpr size_t MASK = Capacity-1;
    public:
        bool push(T&& item);

        std::optional<T> pop();

        bool empty();
        bool full();

        size_t size();
};


template<typename T, size_t Capacity>
bool CircularQueue<T, Capacity>::push(T&& item){
    // take the tail cursor element , and make the order of it relaxed
    const size_t currentTail = tail.load(std::memory_order_relaxed);
    // get the nextTail, and if capacity is full then mask it to be in circular
    const size_t nextTail = (currentTail+1) & MASK;

    // loading the head curosr data with memory order acquire or with ordering
    if(nextTail == head.load(std::memory_order_acquire)){
        return false; // queue is full
    }

    // moving item to tail cursor poiting box of buffer
    buffer[currentTail] = std::move(item);

    // after moving the item , now releasing the acquired ordering from next tail
    tail.store(nextTail, std::memory_order_release);

    return true;
}

template<typename T, size_t Capacity>
std::optional<T> CircularQueue<T,Capacity>::pop(){

    const size_t currentHead = head.load(std::memory_order_relaxed);

    if(currentHead == tail.load(std::memory_order_acquire)){
        return std::nullopt; // queue is empty
    }
    
    T value = std::move(buffer[currentHead]);
    head.store((currentHead+1)&MASK,std::memory_order_release);
    return value;
}

template<typename T, size_t Capacity>
bool CircularQueue<T, Capacity>::empty(){
    return head.load(std::memory_order_acquire) == tail.load(std::memory_order_acquire);
}

template<typename T, size_t Capacity>
bool CircularQueue<T, Capacity>::full(){
    const size_t nextTail = (tail.load(std::memory_order_acquire) + 1) & MASK;

    return nextTail == head.load(std::memory_order_acquire);
}

template<typename T, size_t Capacity>
size_t CircularQueue<T, Capacity>::size(){
    const size_t h = head.load(std::memory_order_acquire);
    const size_t t = tail.load(std::memory_order_acquire);
    return (t-h) & MASK;
}
