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
        ThreadPool pool{ 4 }; // ����һ���� 4 ���̵߳��̳߳�
        std::vector<std::future<int>> futures; // future ���ϣ���ȡ����ֵ
        for (int i = 0; i < 10; ++i) {
            futures.emplace_back(pool.submit(print_task, i));
        }
        for (int i = 0; i < 10; ++i) {
            futures.emplace_back(pool.submit(print_task2, i));
        }
        int sum = 0;
        for (auto& future : futures) {
            sum += future.get(); // get() ��Ա���� ����������ִ����ϣ���ȡ����ֵ
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