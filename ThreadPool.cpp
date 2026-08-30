/*
Implementation of ThreadPool -- we define the number of threads so we can avoid creating them again and again for every small task

*/

#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <chrono>


class ThreadPool {
public:
    ThreadPool(size_t num_workers);
    ~ThreadPool();

    
};
