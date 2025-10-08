#include <iostream>
#include <syncstream>
#include <optional>
#include "ThreadPool.hpp"
#include "Function.hpp"
#include "common_type.hpp"
#include "UniquePtr.hpp"
#include "Array.hpp"
#include "Vector.hpp"
#include "List.hpp"
#include "Optional.hpp"
#include "SharedPtr.hpp"

struct Student {
    const char* name;
    int age;
    explicit Student(const char* name_, int age_) : name{ name_ }, age{ age_ } {
        std::cout << "����" << std::endl;
    }
    Student(Student&&) = delete;
    ~Student() {
        std::cout << age <<"����" << std::endl;
    }
};

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
    {
        struct Animal {
        public:
        };
        struct cat : Animal {

        };
        struct dog :Animal {

        }; 
       
        using what = common_type<Animal, cat>::type;
        using what1 = common_type<int, double, float>::type;
        
    }
    {
        #pragma warning(disable: 4996)  // ����4996�ž���
        auto b = UniquePtr<int[]>(new int[2]);
      
        struct MyClass {
            int a, b, c;
        };
        auto c = makeUnique<MyClass>(1, 2, 3);
         
        std::cout << c->a << "\n";
  
    }
    {
        Array<int, 32> array{1,2,3};
        array.at(0);
        std::cout << array.front() << std::endl;
        array.fill(5);
        std::cout << array.front() << std::endl;
        Array a{ 1,2,3,4 };

    }
    {
        Vector<int> arr;
        for (size_t i = 0; i < 6; i++) {
            arr.push_back(i);
        }
        for (size_t i = 0; i < arr.size(); i++) {
            std::cout << arr[i] << std::endl;
        }
        Vector<int> arr2{ 1,2,3,4,8,9};
        for (size_t i = 0; i < arr2.size(); i++) {
            std::cout <<"��ʼ���б�" << arr2[i] << std::endl;
        }
    }
    {
        List<int> list{1,2,3,4};
        list.push_back(1);
        list.emplace_front(1);
        size_t i = 0;
        list.remove(1);
       
        for (auto it = list.crbegin(); it != list.crend(); ++it) {
            std::cout << "val:" << *it << std::endl;
        }
        std::cout << "list size:" << list.size() << std::endl;
    }
    {
        Optional<int> opt;
        std::cout << opt.value_or(1) << std::endl;
        Optional opt2(1);
    }
    {
        SharedPtr<Student> p = makeShared<Student>("Shaw", 23);
        SharedPtr<Student> p2(new Student("mike", 22));
        SharedPtr<Student> p3 = p2;
        auto p4 = makeShared<int[]>(5);
      
        std::cout << "������" << p->name << " " << "����:" << p->age << std::endl;
        std::cout << "������" << p2->name << " " << "����:" << p2->age << std::endl;
        std::cout << "������" << p3->name << " " << "����:" << p3->age << std::endl;
        return 0;
    }
    
} 