#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

class ThreadPool {
public:
    ThreadPool(size_t);
    ~ThreadPool();
    void Enqueue(std::function<void()>);

private:
    std::vector<std::thread> Workers;
    std::queue<std::function<void()>> Tasks;
    std::mutex QueueMutex;
    std::condition_variable Condition;
    std::atomic<bool> Stop;
};

