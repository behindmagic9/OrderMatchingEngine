#pragma once

#include<mutex>
#include<queue>
#include<optional>
#include<stdexcept>
#include<condition_variable>

// its  a multiple consumer and single consumer (MPSC) queue
template<typename T>
class TQueue {
    std::queue<T> qe;
    std::mutex mx;
    std::condition_variable cv;
    bool closed = false;

public:
    //template<typename U>
    void push(T&& value);

    std::optional<T> pop();

    void close();
};

template<typename T>
//template<typename U>
void TQueue<T>::push(T&& value) {
    {
        std::lock_guard<std::mutex> lock(mx);
        if (closed) {
            throw std::runtime_error("queue is closed no push now");
        }
        // if we have any functiuon with other templkate apramter then we had use std::forward<U> instead of std::move ,
        //right now its using class tempalte T so , can use std::move
        qe.push(std::move(value));
    }
    cv.notify_one();
}

template<typename T>
std::optional<T> TQueue<T>::pop() {
    std::unique_lock<std::mutex> lock(mx);

    cv.wait(lock, [&] {
        return closed || !qe.empty();
        });

    if (closed && qe.empty()) {
        return std::nullopt;
    }

    T temp = std::move(qe.front());
    qe.pop();
    return std::move(temp);
}

template<typename T>
void TQueue<T>::close() {
    {
        std::lock_guard<std::mutex> lock(mx);
        closed = true;
    }
    cv.notify_all();
}