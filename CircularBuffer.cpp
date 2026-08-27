/*
The example: an IMU feeding a control loop

A robot's IMU driver thread produces orientation readings at ~1 kHz. A control loop thread consumes them to update state.
The two run on different cores at different, drifting rates, so you decouple them with a bounded ring buffer: the IMU
pushes readings in, the control loop pops them out, and if the control loop briefly stalls, the buffer absorbs the backlog
up to its capacity
*/

#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <vector>

struct Reading {
    int seq;        // sequence number
    double gyro_z;  // gyro reading in rad/s
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
    std::vector<T> buffer_;
    size_t head_ = 0;
    size_t tail_ = 0;
    size_t size_ = 0;
    size_t capacity_;
    mutable std::mutex mutex_;
};

int main() {
    RingBuffer<Reading> rb(8); // Capacity 8


    std::thread producer([&] {
        for (int i = 0; i < 20; ++i) {
            rb.push({i, 0.1 * i});
            std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Simulate 1 kHz IMU
        }
    });

    std::thread consumer([&] {
        for (int i = 0; i < 20; ++i) {
            Reading r;
            rb.pop(r);
            std::cout << "Consumed reading: seq=" << r.seq << ", gyro_z=" << r.gyro_z << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(3)); // Simulate slower control loop
        }
    });

    producer.join();
    consumer.join();

    return 0;
}
