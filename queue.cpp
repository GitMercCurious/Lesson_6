#include <iostream>
#include <iomanip>
#include <queue>
#include <thread>
#include <mutex>
#include <sstream>

template<typename T>
class queueS : public std::queue<T>{
public:
    void pushS(const T value){
        std::lock_guard<std::mutex> lock(this->changes);
        this->push(value);
    }

    void popS() {
        std::lock_guard<std::mutex> lock(this->changes);
        this->pop();
    }

    void swapS(queueS& other){
        std::scoped_lock lock(this->changes, other.changes);
        this->swap(other);
    }

    template<class ... Args>
    void emplaceS(Args ... args){
        std::lock_guard<std::mutex> lock(this->changes);
        this->emplace(args...);
    }

private:
    std::mutex changes;
};


