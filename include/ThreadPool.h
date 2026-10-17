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

class ThreadPool {
public:
    // 构造函数：默认线程数 = CPU 核心数
    explicit ThreadPool(size_t threads = std::thread::hardware_concurrency())
        : stop_(false)
    {
        if (threads == 0) threads = 4;  // 兜底

        for (size_t i = 0; i < threads; ++i) {
            workers_.emplace_back([this] {
                // 工作线程主循环
                while (true) {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(queue_mutex_);
                        // 等待：有任务 或 线程池停止
                        condition_.wait(lock, [this] {
                            return stop_ || !tasks_.empty();
                        });

                        // 停止且任务已清空 → 线程退出
                        if (stop_ && tasks_.empty()) return;

                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    // 在锁外执行任务，避免长任务阻塞其他线程取任务
                    task();
                }
            });
        }
    }

    // 提交任务，返回 std::future 以获取执行结果
    template <class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result_t<F, Args...>>
    {
        using return_type = typename std::invoke_result_t<F, Args...>;

        // 用 packaged_task 包装任务，以便获取 future
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

        condition_.notify_one();  // 唤醒一个空闲工作线程
        return res;
    }

    // 析构函数：停止所有线程并等待它们结束
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            stop_ = true;
        }
        condition_.notify_all();  // 唤醒所有线程检查停止标志

        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    // 禁止拷贝
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

private:
    std::vector<std::thread> workers_;           // 工作线程容器
    std::queue<std::function<void()>> tasks_;     // 任务队列
    std::mutex queue_mutex_;                      // 保护任务队列的互斥锁
    std::condition_variable condition_;           // 条件变量：通知有新任务
    std::atomic<bool> stop_;                      // 停止标志
};