#include <iostream>
#include <syncstream>
#include "ThreadPool.h"
#include "Function.h"

int print_task(int n) {
    std::osyncstream{ std::cout } << "task" << n << "is running on thr" <<
        std::this_thread::get_id() << "\n";
    return n;
}

int print_task2(int n) {
    std::osyncstream{ std::cout } << "@@@@" << n << "%%%%" << std::endl;
    return n;
}

int main() {
    {
        ThreadPool pool{ 4 }; // 创建一个有 4 个线程的线程池
        std::vector<std::future<int>> futures; // future 集合，获取返回值
        for (int i = 0; i < 10; ++i) {
            futures.emplace_back(pool.submit(print_task, i));
        }
        for (int i = 0; i < 10; ++i) {
            futures.emplace_back(pool.submit(print_task2, i));
        }
        int sum = 0;
        for (auto& future : futures) {
            sum += future.get(); // get() 成员函数 阻塞到任务执行完毕，获取返回值
        }
        std::cout << "sum: " << sum << '\n';
    }
    {
        Function<int(int)> func = print_task2;
        auto f1 = func(2);
        std::cout << f1 << std::endl;
    }
 
    return 0;
} 