# C++11 手写线程池 (ThreadPool)

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![CMake](https://img.shields.io/badge/CMake-3.16%2B-brightgreen.svg)](https://cmake.org/)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20WSL2-orange.svg)]()
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Build](https://img.shields.io/badge/build-passing-success.svg)]()

> 一个从零手写的 ls ~/.ssh/id_ed25519.pub。

---

## 项目简介

本项目是一份**教学与工程并重**的线程池实现，完全基于 C++ 标准库（`<thread>`、`<mutex>`、`<condition_variable>`、`<future>`）手写，不依赖任何第三方库。

**解决的问题**：

- 频繁创建/销毁线程带来的系统开销（每次 `pthread_create` 约几十微秒 + 内核栈分配）。
- 手动管理线程生命周期容易出错、难以优雅退出。
- 需要一个简单的任务调度抽象，把“提交任务”和“任务执行”解耦。

**亮点**：

- 核心代码不足 100 行，注释详尽，适合作为多线程编程的入门读物。
- 提供**三个版本**：基础版、单例版、优先级队列版，覆盖常见工程场景。
- 完整的 CMake 构建、测试用例、调试与 FAQ 文档。

---

## 功能特性

- ✅ **线程复用**：一次性创建 N 个工作线程，长期存活，避免反复创建销毁。
- ✅ **任务队列**：基于 `std::queue<std::function<void()>>` 的线程安全队列。
- ✅ **异步返回值**：`enqueue()` 返回 `std::future<T>`，可获取任务执行结果。
- ✅ **可变参数模板**：支持任意签名的可调用对象和任意数量参数。
- ✅ **优雅关闭**：析构时唤醒所有线程、等待任务清空、`join` 所有工作线程。
- ✅ **CPU 自适应**：默认线程数 = `std::thread::hardware_concurrency()`。
- ✅ **禁止拷贝**：避免多份 `mutex`/`condition_variable` 状态导致未定义行为。
- ✅ **单例模式**（扩展）：Meyers Singleton，C++11 保证线程安全。
- ✅ **优先级队列**（扩展）：`enqueue(priority, f, args...)`，数值越大越先执行。
- ✅ **跨平台**：Linux / WSL2 / macOS 均可编译（CMake 自动链接 pthread）。

---

## 环境依赖

| 项目 | 最低版本 | 推荐版本 |
|------|----------|----------|
| 操作系统 | Ubuntu 20.04 / WSL2 | Ubuntu 24.04 LTS |
| 编译器 | GCC 9 / Clang 10 | GCC 13+ |
| C++ 标准 | C++17 | C++17 |
| CMake | 3.16 | 3.28+ |
| 构建工具 | make / ninja | make |
| 调试器（可选） | GDB 9+ | GDB 14+ |
| 内存检测（可选） | Valgrind 3.15+ | Valgrind 3.22+ |

**无第三方库依赖**，仅需 C++ 标准库。

---

## 安装 / 编译步骤

### 1. 安装工具链（Ubuntu / WSL2）

```bash
sudo apt update
sudo apt install -y build-essential cmake git gdb valgrind
```

### 2. 克隆项目

```bash
git clone https://github.com/MisakaMikoto-QAQ/threadpool.git
cd threadpool
```

> 或者你是在本地从零创建的：`mkdir -p ~/projects/threadpool && cd ~/projects/threadpool`

### 3. 构建（Release）

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### 4. 构建（Debug，用于调试）

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
```

构建成功后，`build/` 目录下会生成三个可执行文件：

```
build/threadpool_test              # 基础版测试
build/threadpool_singleton_test    # 单例版测试
build/threadpool_priority_test     # 优先级版测试
```

---

## 快速开始 / 使用示例

**最小可运行示例（5 行核心代码）**：

```cpp
#include "ThreadPool.h"
#include <iostream>

int main() {
    ThreadPool pool(4);                          // 4 个工作线程

    auto future = pool.enqueue([](int x) {       // 提交任务
        return x * x;
    }, 7);

    std::cout << "Result = " << future.get()     // 阻塞获取结果
              << std::endl;                      // 输出 49
    return 0;
}
```

编译运行：

```bash
g++ -std=c++17 -Iinclude -pthread src/main.cpp -o demo && ./demo
```

或者直接用 CMake：

```bash
cmake -B build && cmake --build build
./build/threadpool_test
```

---

## 项目结构

```
threadpool/
├── CMakeLists.txt                  # CMake 构建配置
├── README.md                       # 本文件
├── LICENSE                         # MIT 许可证
├── include/
│   ├── ThreadPool.h                # 基础版线程池（核心）
│   ├── ThreadPoolSingleton.h       # 单例版（扩展）
│   └── ThreadPoolPriority.h        # 优先级队列版（扩展）
├── src/
│   ├── main.cpp                    # 基础版测试程序
│   ├── test_singleton.cpp          # 单例版测试程序
│   └── test_priority.cpp           # 优先级版测试程序
└── build/                          # 构建产物（cmake -B build 生成，不提交到 git）
```

**关键文件说明**：

| 文件 | 作用 |
|------|------|
| `include/ThreadPool.h` | 线程池核心类，包含任务队列、工作线程、条件变量 |
| `src/main.cpp` | 演示任务提交、返回值获取、并发性能验证 |
| `CMakeLists.txt` | 定义三个可执行目标，自动链接 `Threads::Threads` |

---

## 核心设计 / 原理说明

### 整体架构

```
         ┌──────────────────┐
         │   主线程 / 生产者  │
         └────────┬─────────┘
                  │ enqueue(f, args...)
                  ▼
         ┌──────────────────┐
         │  任务队列 (queue) │  ←──┐
         └────────┬─────────┘     │ 加锁保护
                  │               │
        notify_one() 唤醒         │
                  ▼               │
    ┌───────────────────────────┐ │
    │ 工作线程 W1..Wn (workers) │─┘
    │  wait() 阻塞 → pop() → 执行│
    └───────────────────────────┘
```

### 三大核心组件

**1. 任务队列 `std::queue<std::function<void()>>`**

- 使用 `std::function` 做类型擦除，任何签名兼容的可调用对象（lambda、函数指针、`std::bind` 结果）都能入队。
- 配合 `std::packaged_task` 与 `std::shared_ptr`，实现跨线程返回值传递。

**2. 条件变量 `std::condition_variable`**

工作线程主循环：

```cpp
std::unique_lock<std::mutex> lock(queue_mutex_);
condition_.wait(lock, [this] {
    return stop_ || !tasks_.empty();   // 谓词，防虚假唤醒
});
if (stop_ && tasks_.empty()) return;   // 停止条件
task = std::move(tasks_.front());
tasks_.pop();
```

- 使用带谓词的 `wait()`，避免因虚假唤醒或 `notify` 丢失导致的死锁。
- **必须在锁外执行 `task()`**，否则长任务会阻塞其他线程取任务。

**3. 停止机制（优雅关闭）**

```cpp
~ThreadPool() {
    { std::unique_lock<std::mutex> lock(queue_mutex_); stop_ = true; }
    condition_.notify_all();               // 唤醒所有等待线程
    for (auto& w : workers_) w.join();     // 等待线程退出
}
```

- 采用 `stop_` 标志 + `notify_all()`，保证**已入队任务全部执行完**才退出（软关闭）。
- 使用 `std::atomic<bool>` 保证标志的可见性。

### 关键设计决策

| 决策 | 原因 |
|------|------|
| 用 `std::function` 而非函数指针 | 支持 lambda 捕获、`std::bind` |
| 用 `std::packaged_task` | 无需自定义任务基类，即可实现 `future` 返回值 |
| 用 `std::shared_ptr<packaged_task>` | 保证任务在 lambda 被销毁后仍存活 |
| 用 `invoke_result_t` 而非 `result_of` | C++17 中 `result_of` 已弃用 |
| `Threads::Threads`（CMake） | 跨平台链接线程库的正确方式 |

---

## API 说明

### `class ThreadPool`

#### 构造函数

```cpp
explicit ThreadPool(size_t threads = std::thread::hardware_concurrency());
```

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `threads` | `size_t` | CPU 核心数 | 工作线程数量；传入 0 时兜底为 4 |

**异常**：若线程创建失败（`std::system_error`），构造函数会抛出异常，此时已创建的部分线程会在析构时 `join`（实际场景中建议在构造函数中捕获并清理，本教学版未做）。

---

#### `enqueue` — 提交任务

```cpp
template <class F, class... Args>
auto enqueue(F&& f, Args&&... args)
    -> std::future<std::invoke_result_t<F, Args...>>;
```

| 参数 | 类型 | 说明 |
|------|------|------|
| `f` | 可调用对象 | 支持函数指针、lambda、`std::function` 等 |
| `args...` | 任意类型 | 会以完美转发方式传递给 `f` |

**返回值**：`std::future<R>`，其中 `R = std::invoke_result_t<F, Args...>`。可通过 `.get()` 阻塞获取结果，或 `.wait_for()` 超时检测。

**异常**：

- `std::runtime_error("enqueue on stopped ThreadPool")`：对已析构的线程池提交任务。
- `std::future_error`：若同一 `future` 被多次 `.get()`。

**示例**：

```cpp
ThreadPool pool(4);

// 无返回值
pool.enqueue([]{ std::cout << "hello\n"; });

// 有返回值
auto f1 = pool.enqueue([](int a, int b) { return a + b; }, 3, 4);
std::cout << f1.get();  // 7

// 传引用（注意使用 std::ref）
int x = 10;
auto f2 = pool.enqueue([](int& v) { v *= 2; }, std::ref(x));
f2.get();  // x 变为 20
```

---

#### 析构函数

```cpp
~ThreadPool();
```

- 设置 `stop_ = true`，唤醒所有等待线程。
- 所有已入队任务**会先执行完**，然后线程退出。
- 阻塞直到所有 `join()` 完成。

**异常**：析构函数不抛异常；若在工作线程中调用任务时抛出异常，会导致 `std::terminate`（教学版未做异常隔离，生产环境建议在 `task()` 外层 `try/catch`）。

---

#### 拷贝/移动

```cpp
ThreadPool(const ThreadPool&) = delete;
ThreadPool& operator=(const ThreadPool&) = delete;
```

**不可拷贝、不可移动**：`std::mutex` 和 `std::condition_variable` 本身不可复制，强行复制会造成多个对象共享底层状态。

---

### `class ThreadPoolSingleton`（扩展）

```cpp
static ThreadPoolSingleton& getInstance(
    size_t threads = std::thread::hardware_concurrency());
```

- 首次调用时构造单例，后续参数被忽略。
- 基于 Meyers Singleton，线程安全由 C++11 标准保证。
- 生命周期持续到程序退出（静态对象析构顺序）。

---

### `class ThreadPoolPriority`（扩展）

```cpp
template <class F, class... Args>
auto enqueue(int priority, F&& f, Args&&... args)
    -> std::future<std::invoke_result_t<F, Args...>>;
```

| 参数 | 说明 |
|------|------|
| `priority` | 整数，**越大越优先**（默认 0） |
| `f, args...` | 同基础版 |

**注意**：优先级决定**取出顺序**，多线程执行时序仍受 OS 调度影响。

---

## 运行测试

### 编译三个测试目标

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
```

### 运行全部测试

```bash
echo "===== Basic =====";      ./build/threadpool_test
echo "===== Singleton =====";  ./build/threadpool_singleton_test
echo "===== Priority =====";   ./build/threadpool_priority_test
```

### 预期结果

**基础版 `threadpool_test`**：

- 测试 1：6 个无返回值任务被打散到 4 个不同线程 ID 上。
- 测试 2：`heavy_calculation(3) = 9` 等结果正确。
- 测试 3：8 个 300ms 任务在 4 线程池中总耗时 ≈ 600ms（而非 2400ms）。

**单例版 `threadpool_singleton_test`**：

- 所有线程打印的实例地址**完全一致**。
- `Same instance? YES`。

**优先级版 `threadpool_priority_test`**：

```
[P5] HIGH priority task B
[P5] HIGH priority task E
[P3] mid priority task C
[P2] low-mid task F
[P1] low priority task A
[P1] low priority task D
```

### 使用 Valgrind 检测内存泄漏（可选）

```bash
valgrind --leak-check=full --error-exitcode=1 ./build/threadpool_test
```

**预期**：`ERROR SUMMARY: 0 errors from 0 contexts`，且 `All heap blocks were freed`。

---

## 常见问题 FAQ

**Q1: 编译报错 `undefined reference to 'pthread_create'`**

A: 未链接线程库。检查 `CMakeLists.txt` 是否包含：

```cmake
find_package(Threads REQUIRED)
target_link_libraries(threadpool_test PRIVATE Threads::Threads)
```

或者手动编译时加 `-pthread`：

```bash
g++ -std=c++17 -pthread src/main.cpp -Iinclude -o demo
```

**Q2: `'invoke_result_t' is not a member of 'std'`**

A: 编译器未启用 C++17。检查 `CMakeLists.txt`：

```cmake
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

**Q3: 程序退出时卡住不返回**

A: 通常是死锁，常见原因：

- 在任务里再次调用 `pool.enqueue()` 并 `.get()` 等待结果 → 死锁（工作线程都在等，无人执行新任务）。解决：不要在池内任务中阻塞等待同一池的 future。
- 忘记在析构前调用 `notify_all()`。本项目已在析构函数中处理。

**Q4: 任务中抛异常导致程序崩溃**

A: 教学版未做异常隔离。生产环境建议在线程主循环中包一层 `try/catch`：

```cpp
try { task(); }
catch (const std::exception& e) { /* 记录日志 */ }
```

**Q5: 优先级队列测试输出顺序不稳定？**

A: 使用了多线程池。多线程下 `top()` + `pop()` 是加锁的，但任务**执行顺序**受 OS 调度影响。测试时请用 `ThreadPoolPriority pool(1)`（1 个线程）+ 闸门阻塞法，如本项目的 `test_priority.cpp`。

**Q6: 单例测试中地址不一致？**

A: 构造函数是 `private` 时，编译器会直接阻止外部构造。如果仍出现地址不同，检查是否有 `new ThreadPoolSingleton(...)` 之类的调用（会编译报错），或者存在多个共享库各自持有一份静态变量（一般不会）。

**Q7: 如何控制线程数？**

```cpp
ThreadPool pool(8);                       // 8 线程
ThreadPool pool;                          // 默认 = CPU 核心数
ThreadPool pool(0);                       // 兜底为 4 线程
```

**Q8: Windows 原生 MSVC 能编译吗？**

A: 可以，`Threads::Threads` 在 Windows 上会自动映射到 Win32 线程库，无需额外配置。本项目在 WSL2 + GCC 13 上验证通过，MSVC 2019+ 理论也可。

---

## 贡献指南

### 提交 Issue

提 Issue 前请先搜索是否已有相同问题。请包含：

- 操作系统、编译器版本、CMake 版本（`g++ --version`、`cmake --version`）。
- 完整的错误信息（包括编译命令）。
- 最小可复现示例。

### 提交 Pull Request

1. Fork 本仓库，创建分支：`git checkout -b feature/your-feature`。
2. 保持代码风格：
   - 遵循 **Google C++ Style**（缩进 4 空格、`snake_case_` 成员变量、左花括号不换行）。
   - 所有公共 API 必须写注释。
   - 新增功能需附带测试用例。
3. 通过本地测试：

   ```bash
   cmake -B build && cmake --build build -j
   ./build/threadpool_test && ./build/threadpool_singleton_test && ./build/threadpool_priority_test
   ```

4. 提交 PR，描述**动机、实现方式、测试结果**。
5. 建议开启 `-Wall -Wextra -Wpedantic`，确保无警告。

### 代码风格速查

```cpp
class MyClass {
public:
    void publicMethod();          // 方法：lowerCamelCase 或 snake_case（本项目用 snake_case）
private:
    int member_variable_;         // 成员变量：snake_case + 尾部下划线
    static constexpr int kMax = 10;  // 常量：k 前缀
};
```

---

## 许可证 License

本项目基于 [MIT License](LICENSE) 发布。

```
MIT License

Copyright (c) 2024 <MisakaMikoto-QAQ>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

## 作者与联系方式

- **作者**：MisakaMikoto-QAQ
- **邮箱**：3128204593@qq.com
- **GitHub**：MisakaMikoto-QAQ(https://github.com/MisakaMikoto-QAQ)
- **项目仓库**：https://github.com/MisakaMikoto-QAQ/threadpool

欢迎 Star ⭐、提 Issue、PR！

---

## 致谢 / 参考

- [cppreference.com](https://en.cppreference.com/) — C++ 标准库权威文档。
- 《C++ Concurrency in Action》(2nd Edition) — Anthony Williams，多线程编程经典。
- [progschj/ThreadPool](https://github.com/progschj/ThreadPool) — 本项目 `packaged_task` 方案的灵感来源。
- [Meyers Singleton 讨论](https://stackoverflow.com/questions/1661529/is-meyers-implementation-of-singleton-pattern-thread-safe) — C++11 局部静态变量线程安全。
- GCC 13 官方文档、CMake 3.28 官方文档。

---

## 更新日志 / 路线图

### Changelog

**v1.0.0 (2024-XX-XX)**

- 🎉 首个版本发布。
- 基础版线程池：任务队列、`future` 返回值、优雅关闭。
- 扩展版：单例模式、优先级队列。
- 完整 CMake 构建与测试。

### Roadmap

- [ ] **v1.1** — 动态扩缩容（根据队列长度自动增减线程）。
- [ ] **v1.2** — 任务超时取消（`future.wait_for` + `cancel` 接口）。
- [ ] **v1.3** — 异常隔离：任务异常不会终止工作线程。
- [ ] **v1.4** — 无锁队列版本（`boost::lockfree::queue` 或自研 MPSC 队列）。
- [ ] **v1.5** — 性能基准测试（对比 `std::async`、TBB、OpenMP）。
- [ ] **v2.0** — 支持协程任务（C++20 coroutine）。

---

**如果这个项目对你有帮助，请给个 ⭐ Star！**