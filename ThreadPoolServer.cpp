#include "ThreadPoolServer.h"

ThreadPool::ThreadPool(size_t Threads) : Stop(false) {
    for (size_t i = 0; i < Threads; ++i) {
        Workers.emplace_back([this] {
            while (true) {
                std::function<void()> Task;
                {
                    std::unique_lock<std::mutex> Lock(this->QueueMutex);
                    this->Condition.wait(Lock, [this] {return this->Stop || !this->Tasks.empty();});

                    if (this->Stop && this->Tasks.empty()) {
                        return;
                    }

                    Task = std::move(this->Tasks.front());
                    this->Tasks.pop();
                }
                Task();

            }
        });
    }
}

void ThreadPool::Enqueue(std::function<void()> Task) {
    {
        std::unique_lock<std::mutex> Lock(QueueMutex);
        Tasks.emplace(std::move(Task));
    }
    Condition.notify_one();
}

ThreadPool::~ThreadPool() {
    Stop = true;
    Condition.notify_all();
    for (std::thread &Worker : Workers) {
        Worker.join();
    }
}
