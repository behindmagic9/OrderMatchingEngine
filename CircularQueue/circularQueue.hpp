#pragma once
#include<array>
#include<condition_variable>
#include<mutex>
#include<optional>

template<typename T, size_t Capacity>
class CircularQueue{
    std::array<T, Capacity> buffer;
    size_t head = 0;
    size_t tail = 0;
    size_t count =0;
    std::mutex m;
    std::condition_variable notEmpty;
    std::condition_variable notFull;
    bool closed = false;
    public:

        
        void push(T&& item);

        std::optional<T> pop();

        void close();


        bool empty();
        bool full();

        size_t size();
};


template<typename T, size_t Capacity>
void CircularQueue<T, Capacity>::push(T&& item){
    std::unique_lock<std::mutex> lock(m);

    notFull.wait(lock,[&]{
        return count<Capacity || closed;
    });

    if(closed) return;
    buffer[tail] = std::move(item);
    tail = (tail+1)%Capacity;
    ++count;
    lock.unlock();
    notEmpty.notify_one();
}

template<typename T, size_t Capacity>
std::optional<T> CircularQueue<T,Capacity>::pop(){
    std::unique_lock<std::mutex> lock(m);

    notEmpty.wait(lock,[&]{
        return count>0 || closed;
    });

    if(count ==0 && closed){
        return std::nullopt;
    }
    T value = std::move(buffer[head]);
    head=(head+1)%Capacity;
    --count;
    lock.unlock();
    notFull.notify_one();
    return value;
}

template<typename T, size_t Capacity>
void CircularQueue<T, Capacity>::close(){
    {
        std::lock_guard<std::mutex> lock(m);
        closed = true;
    }
    notEmpty.notify_all();
    notFull.notify_all();

}

template<typename T, size_t Capacity>
bool CircularQueue<T, Capacity>::empty(){
    std::lock_guard<std::mutex> lock(m);
    return count == 0;
}

template<typename T, size_t Capacity>
bool CircularQueue<T, Capacity>::full(){
    std::lock_guard<std::mutex> lock(m);
    return count == Capacity;
}

template<typename T, size_t Capacity>
size_t CircularQueue<T, Capacity>::size(){
    std::lock_guard<std::mutex> lock(m);
    return count;
}
