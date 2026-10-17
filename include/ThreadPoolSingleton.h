#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <atomic>
#include <type_traits>
#include <stdexcept>

class ThreadPoolSingleton {
public:
    // ============ 获取全局唯一实例 ============
    // 注意：threads 参数只在第一次调用时生效，后续调用会被忽略
    static ThreadPoolSingleton& getInstance(
        size_t threads = std::thread::hardware_concurrency())
    {
        static ThreadPoolSingleton instance(threads);  // C++11 保证线程安全
        return instance;
    }

    // ============ 提交任务 ============
    template <class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result_t<F, Args...>>
    {
        using return_type = typename std::invoke_result_t<F, Args...>;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> res = task->get_future();

        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            if (stop_) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }
            tasks_.emplace([task]() { (*task)(); });
        }
        condition_.notify_one();
        return res;
    }

    // ============ 禁止拷贝和移动 ============
    ThreadPoolSingleton(const ThreadPoolSingleton&) = delete;
    ThreadPoolSingleton& operator=(const ThreadPoolSingleton&) = delete;
    ThreadPoolSingleton(ThreadPoolSingleton&&) = delete;
    ThreadPoolSingleton& operator=(ThreadPoolSingleton&&) = delete;

private:
    // ============ 构造函数/析构函数设为私有 ============
    explicit ThreadPoolSingleton(size_t threads) : stop_(false) {
        if (threads == 0) threads = 4;

        for (size_t i = 0; i < threads; ++i) {
            workers_.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex_);
                        condition_.wait(lock, [this] {
                            return stop_ || !tasks_.empty();
                        });
                        if (stop_ && tasks_.empty()) return;
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    task();
                }
            });
        }
    }

    ~ThreadPoolSingleton() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            stop_ = true;
        }
        condition_.notify_all();
        for (auto& worker : workers_) {
            if (worker.joinable()) worker.join();
        }
    }

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_;
};