#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <vector>
#include <condition_variable>

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
        std::unique_lock<std::mutex> lock(mutex_);
        not_full_.wait(lock, [this] { return size_ < capacity_ || done_; });
        if (done_) {
            return false; // Buffer is full and shutdown has been called
        }
        buffer_[tail_] = item;
        tail_ = (tail_ + 1) % capacity_;
        ++size_;
        lock.unlock();
        not_empty_.notify_one();
        return true;
    }
    
    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        not_empty_.wait(lock, [this] { return size_ > 0 || done_; });
        if (size_ == 0 && done_) {
            return false; // Buffer is empty and shutdown has been called
        }
        item = std::move(buffer_[head_]);
        head_ = (head_ + 1) % capacity_;
        --size_;
        lock.unlock();
        not_full_.notify_one();
        return true;
    }

    void shutdown() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            done_ = true;
        }
        not_full_.notify_all();
        not_empty_.notify_all();
    }

private:
    std::vector<T> buffer_;
    size_t capacity_;
    size_t head_ = 0;
    size_t tail_ = 0;
    size_t size_ = 0;
    std::mutex mutex_;
    std::condition_variable not_full_, not_empty_;
    bool done_ = false;
};


class ParserThreadPool {
public:
    ParserThreadPool(size_t num_workers, RingBuffer<Reading>& rb) : rb_(rb) {
        for (size_t i = 0; i < num_workers; ++i) {
            workers_.emplace_back([this] { worker_loop(); });
        }
    }
    ~ParserThreadPool() {
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

private:
    void worker_loop() {
        // Implementation for the worker thread
        while (true) {
            std::cout << "Thread " << std::this_thread::get_id() << " executing task" << std::endl;

            if(rb_.pop(current_reading_)) {
                std::cout << "Thread " << std::this_thread::get_id() << " processed reading: " << current_reading_.seq << std::endl;
            } else {
                std::cout << "Thread " << std::this_thread::get_id() << " stopping as buffer is empty and shutdown has been called." << std::endl;
                break; // Exit the loop if shutdown has been called and buffer is empty
            }

        }
    }

    std::vector<std::thread> workers_;
    RingBuffer<Reading>& rb_;
    Reading current_reading_;
};

void dataGenerator(RingBuffer<Reading>& rb, const std::chrono::steady_clock::time_point start_time) {
    Reading reading;
    reading.seq = 0;
    
    reading.euler_x = 0.0;
    reading.euler_y = 0.0;
    reading.euler_z = 0.0;

    while (std::chrono::steady_clock::now() - start_time < std::chrono::seconds(30)) {
        reading.seq++;
        reading.euler_x = 0.1 * reading.seq;
        reading.euler_y = 0.1 * reading.seq;
        reading.euler_z = 0.1 * reading.seq;
        rb.push(reading);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main() {
    RingBuffer<Reading> rb(8); // Capacity 8
    ParserThreadPool parser_pool(4, rb); // 4 worker threads

    auto start_time = std::chrono::steady_clock::now();

    std::thread reader(dataGenerator, std::ref(rb), start_time);

    reader.join();
    rb.shutdown(); // Signal the parser threads to stop

    return 0;
}