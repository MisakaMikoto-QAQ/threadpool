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

class ThreadPoolPriority {
public:
    explicit ThreadPoolPriority(
        size_t threads = std::thread::hardware_concurrency())
        : stop_(false)
    {
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

                        // 从 priority_queue 取出最高优先级任务
                        // top() 返回 const 引用，用 const_cast 移动 func
                        task = std::move(
                            const_cast<Task&>(tasks_.top()).func
                        );
                        tasks_.pop();
                    }
                    task();
                }
            });
        }
    }

    // ============ 带优先级提交 ============
    // priority: 数值越大越优先（默认 0）
    template <class F, class... Args>
    auto enqueue(int priority, F&& f, Args&&... args)
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
            // 构造 Task 并压入优先队列
            Task t;
            t.priority = priority;
            t.func = [task]() { (*task)(); };
            tasks_.push(std::move(t));
        }
        condition_.notify_one();
        return res;
    }

    // 兼容原接口：不传优先级时使用默认 0
    template <class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result_t<F, Args...>>
    {
        return enqueue(0,
                       std::forward<F>(f),
                       std::forward<Args>(args)...);
    }

    ~ThreadPoolPriority() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            stop_ = true;
        }
        condition_.notify_all();
        for (auto& worker : workers_) {
            if (worker.joinable()) worker.join();
        }
    }

    ThreadPoolPriority(const ThreadPoolPriority&) = delete;
    ThreadPoolPriority& operator=(const ThreadPoolPriority&) = delete;

private:
    // ============ 任务结构体 ============
    struct Task {
        int priority = 0;
        std::function<void()> func;

        // priority_queue 是大顶堆，所以数值大的优先
        // 定义 < 为“优先级更低者排在后面”
        bool operator<(const Task& other) const {
            return priority < other.priority;
        }
    };

    std::vector<std::thread> workers_;
    std::priority_queue<Task> tasks_;             // 改为优先队列
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_;
};