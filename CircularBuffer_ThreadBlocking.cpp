/*
The example: an IMU feeding a control loop

A robot's IMU driver thread produces orientation readings at ~1 kHz. A control loop thread consumes them to update state.
The two run on different cores at different, drifting rates, so you decouple them with a bounded ring buffer: the IMU
pushes readings in, the control loop pops them out, and if the control loop briefly stalls, the buffer absorbs the backlog
up to its capacity

This is the block version, which is more complex and slower, but allows the producer and consumer to run on the same core.
*/

#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <vector>
#include <condition_variable>

struct Reading {
    int seq;        // sequence number
    double gyro_z;  // gyro reading in rad/s
};

template <typename T>
class RingBuffer {
public:
    RingBuffer(size_t capacity) : buffer_(capacity), capacity_(capacity) {}
    
    void push(const T& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        not_full_.wait(lock, [this] { return size_< capacity_; });
        buffer_[tail_] = item;
        tail_ = (tail_ + 1) % capacity_;
        ++size_;
        lock.unlock();
        not_empty_.notify_one();
    }
    
    void pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        not_empty_.wait(lock, [this] { return size_ > 0; });
        item = std::move(buffer_[head_]);
        head_ = (head_ + 1) % capacity_;
        --size_;
        lock.unlock();
        not_full_.notify_one();
    }
    
private:
    std::vector<T> buffer_;
    size_t head_ = 0;
    size_t tail_ = 0;
    size_t size_ = 0;
    size_t capacity_;
    mutable std::mutex mutex_;
    std::condition_variable not_full_, not_empty_;
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
