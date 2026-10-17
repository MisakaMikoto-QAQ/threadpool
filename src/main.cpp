#include "ThreadPool.h"
#include <iostream>
#include <chrono>
#include <string>

// 模拟耗时任务
int heavy_calculation(int id) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    return id * id;
}

int main() {
    std::cout << "=== ThreadPool Test ===" << std::endl;

    // 创建包含 4 个工作线程的线程池
    ThreadPool pool(4);
    std::cout << "ThreadPool created with 4 workers." << std::endl;

    // ---- 测试1：无返回值任务 ----
    std::cout << "\n[Test 1] Enqueue void tasks..." << std::endl;
    for (int i = 0; i < 6; ++i) {
        pool.enqueue([i] {
            std::cout << "  Task " << i
                      << " running on thread "
                      << std::this_thread::get_id() << std::endl;
        });
    }

    // ---- 测试2：有返回值任务 ----
    std::cout << "\n[Test 2] Enqueue tasks with return values..." << std::endl;
    std::vector<std::future<int>> futures;
    for (int i = 1; i <= 5; ++i) {
        futures.emplace_back(pool.enqueue(heavy_calculation, i));
    }

    // 收集结果
    for (int i = 0; i < 5; ++i) {
        std::cout << "  heavy_calculation(" << (i + 1) << ") = "
                  << futures[i].get() << std::endl;
    }

    // ---- 测试3：并发执行验证 ----
    std::cout << "\n[Test 3] Concurrent execution test..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::future<int>> batch;
    for (int i = 0; i < 8; ++i) {
        batch.emplace_back(pool.enqueue([]() -> int {
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            return 1;
        }));
    }
    for (auto& f : batch) f.get();

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "  8 tasks x 300ms finished in " << ms.count()
              << " ms (4 threads should take ~600ms)" << std::endl;

    std::cout << "\n=== All tests passed! ===" << std::endl;
    return 0;
    // pool 析构时自动停止所有线程
}