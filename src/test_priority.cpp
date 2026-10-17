#include "ThreadPoolPriority.h"
#include <iostream>
#include <vector>
#include <mutex>
#include <string>

std::mutex io_mutex;   // 保证日志不交错

void log(const std::string& msg) {
    std::lock_guard<std::mutex> lock(io_mutex);
    std::cout << msg << std::endl;
}

int main() {
    std::cout << "=== Priority ThreadPool Test ===" << std::endl;

    // 只开 1 个线程，保证任务严格串行执行，方便观察优先级顺序
    // 多线程下观察顺序会受调度影响，不利于验证
    ThreadPoolPriority pool(1);

    // 先阻塞住这个唯一的工作线程，让后面的任务在队列里排队
    std::promise<void> gate;
    std::future<void> gate_future = gate.get_future();
    pool.enqueue(0, [&gate_future] {
        gate_future.wait();  // 等待放行
    });

    // 让主线程稍微等一会，确保上面的阻塞任务已经被取走
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // ---- 提交不同优先级的任务 ----
    // 注意：此时工作线程被阻塞，任务会全部堆积在优先队列中
    pool.enqueue(1, [] { log("  [P1] low priority task A"); });
    pool.enqueue(5, [] { log("  [P5] HIGH priority task B"); });
    pool.enqueue(3, [] { log("  [P3] mid priority task C"); });
    pool.enqueue(1, [] { log("  [P1] low priority task D"); });
    pool.enqueue(5, [] { log("  [P5] HIGH priority task E"); });
    pool.enqueue(2, [] { log("  [P2] low-mid task F"); });

    log("\nAll tasks enqueued. Releasing the gate...\n");

    // 放行阻塞任务，让工作线程开始消费队列
    gate.set_value();

    // 等待所有任务执行完成（用一个低优先级任务作为“尾哨兵”）
    std::promise<void> done;
    auto done_future = done.get_future();
    // 尾哨兵优先级设为最低 -1，会排在最后执行
    pool.enqueue(-1, [&done] { done.set_value(); });
    done_future.wait();

    std::cout << "\n=== Test finished ===" << std::endl;
    return 0;
}