#include "ThreadPoolSingleton.h"
#include <iostream>
#include <vector>
#include <atomic>

std::atomic<int> task_count{0};

void worker_task(int id) {
    auto& pool = ThreadPoolSingleton::getInstance();
    std::cout << "Thread " << id
              << " got instance at address: " << &pool << std::endl;
    task_count++;
}

int main() {
    std::cout << "=== Singleton ThreadPool Test ===" << std::endl;

    // 方式1：主线程获取实例
    auto& pool = ThreadPoolSingleton::getInstance(4);
    std::cout << "Main got instance at address: " << &pool << std::endl;

    // 方式2：从多个线程并发调用 getInstance，验证是否返回同一对象
    std::vector<std::thread> callers;
    for (int i = 0; i < 8; ++i) {
        callers.emplace_back(worker_task, i);
    }
    for (auto& t : callers) t.join();

    // 方式3：提交任务
    std::cout << "\nSubmitting tasks via singleton..." << std::endl;
    std::vector<std::future<int>> futures;
    for (int i = 0; i < 5; ++i) {
        futures.emplace_back(pool.enqueue([i] {
            return i * 10;
        }));
    }
    for (auto& f : futures) {
        std::cout << "  Result: " << f.get() << std::endl;
    }

    // 再次获取实例，验证是同一个对象
    auto& pool2 = ThreadPoolSingleton::getInstance();
    std::cout << "\nSecond getInstance address: " << &pool2 << std::endl;
    std::cout << "Same instance? " << (&pool == &pool2 ? "YES" : "NO") << std::endl;

    return 0;
}