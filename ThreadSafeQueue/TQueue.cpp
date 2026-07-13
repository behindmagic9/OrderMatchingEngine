#include<mutex>
#include<queue>
#include<optional>
#include<stdexcept>
#include<condition_variable>

// its  a multiple consumer and single consumer (MPSC) queue
template<typename T>
class TQueue{
    std::queue<T> qe;
    std::mutex mx;
    std::condition_variable cv;
    bool closed = false;

    public:
        void push(T value){
            {
                std::lock_guard<std::mutex> lock(mx);
                if(closed){
                    throw std::__throw_runtime_error("queue is closed no push now");
                }
                qe.push(std::move(value));
            }
            cv.notify_one();
        }

        std::optional<T> pop(){
            std::unique_lock<std::mutex> lock(mx);

            cv.wait(lock, [&]{
                return closed || !qe.empty();
            });

            if(closed  && qe.empty()){
                return std::nullopt;
            }

            T temp = std::move(qe.front());
            qe.pop();
            return std::move(temp);
        }

        void close(){
            {
                std::lock_guard<std::mutex> lock(mx);
                closed = true;
            }
            cv.notify_all();
        }
};
