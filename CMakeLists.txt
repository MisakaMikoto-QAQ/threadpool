cmake_minimum_required(VERSION 3.16)
project(ThreadPoolProject VERSION 1.0.0 LANGUAGES CXX)

# C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 头文件目录
include_directories(${CMAKE_SOURCE_DIR}/include)

# 可执行文件
add_executable(threadpool_test
    src/main.cpp
)

# 链接线程库（Linux 下必须）
find_package(Threads REQUIRED)
target_link_libraries(threadpool_test PRIVATE Threads::Threads)

# 可选：编译时显示更多警告
target_compile_options(threadpool_test PRIVATE
    -Wall -Wextra -Wpedantic
)

# 可选：Debug 模式下生成调试信息
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(threadpool_test PRIVATE -g -O0)
endif()
# 单例测试程序
add_executable(threadpool_singleton_test
    src/test_singleton.cpp
)
target_link_libraries(threadpool_singleton_test PRIVATE Threads::Threads)
target_compile_options(threadpool_singleton_test PRIVATE -Wall -Wextra -Wpedantic)
# 优先级队列测试程序
add_executable(threadpool_priority_test
    src/test_priority.cpp
)
target_link_libraries(threadpool_priority_test PRIVATE Threads::Threads)
target_compile_options(threadpool_priority_test PRIVATE -Wall -Wextra -Wpedantic)