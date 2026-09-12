#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <vector>

struct Reading {
    int seq;        // sequence number
    double  euler_x;
    double euler_y;
    double euler_z;
};

struct WriteData {
    int seq;        // sequence number
    double quaternion_w;
    double quaternion_x;
    double quaternion_y;
    double quaternion_z;
};

template <typename T>
class RingBuffer {
public:
    RingBuffer(size_t capacity) : buffer_(capacity), capacity_(capacity) {}
    
    bool push(const T& item) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (size_ == capacity_) {
            return false; // Buffer is full
        }
        buffer_[tail_] = item;
        tail_ = (tail_ + 1) % capacity_;
        ++size_;
        return true;
    }
    
    bool pop(T& item) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (size_ == 0) {
            return false; // Buffer is empty
        }
        item = std::move(buffer_[head_]);
        head_ = (head_ + 1) % capacity_;
        --size_;
        return true;
    }

private:
    std::vector<int> buffer_;
    size_t capacity_;
    size_t head_ = 0;
    size_t tail_ = 0;
    size_t size_ = 0;
    std::mutex mutex_;
};

int main() {
    RingBuffer<Reading> rb(8); // Capacity 8


    return 0;
}