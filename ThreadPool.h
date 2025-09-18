#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <functional>
#include <future>
#include <memory>

inline std::size_t default_thread_pool_size() noexcept {
	std::size_t num_threads = std::thread::hardware_concurrency();
	num_threads = num_threads == 0 ? 2 : num_threads; // 防止无法检测当前硬件，保证线程池至少有两个线程
	return num_threads;
}

class ThreadPool {
public:
	using Task = std::packaged_task<void()>;

	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

	ThreadPool(std::size_t num_thread = default_thread_pool_size()) :
		stop_{ false }, num_threads_{ num_thread }
	{
		// start函数
		start();
	}
	~ThreadPool() {
		stop();
	}

	// 启动函数，添加任务函数
	template<typename F, typename ...Args>
	std::future<std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>> submit(F&& f, Args&&... args) {
		using RetType = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>;
		if (stop_) {
			throw std::runtime_error("ThreadPool is stopped");
		}
		auto task = std::make_shared<std::packaged_task<RetType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
		std::future<RetType> ret = task->get_future();
		{
			std::lock_guard<std::mutex> lc{ mutex_ };
			tasks_.emplace([task] {(*task)(); }); // 智能指针重载*返回对应类型引用 ，packaged_task重载（）表示调用
		}
		cv_.notify_one();
		return ret;
	}
	void start() {
		for (std::size_t i = 0; i < num_threads_; ++i) {
			pool_.emplace_back([this]
				{
					while (!stop_)
					{
						Task task;
						{
							std::unique_lock<std::mutex> lock{ mutex_ };
							cv_.wait(lock, [this] {return stop_ || !tasks_.empty(); });
							if (tasks_.empty())  return;
							task = std::move(tasks_.front());
							tasks_.pop();
						}
						task();
					}
			});
		}
	}
	void stop() {
		stop_.store(true);
		cv_.notify_all();
		for (auto& thread : pool_) {
			if (thread.joinable()) {
				thread.join();
			}
		}
		pool_.clear();
	}

private:
	std::mutex               mutex_;         // 互斥量
	std::condition_variable  cv_;            // 条件变量
	std::atomic<bool>        stop_;          // 指示线程池是否停止   
	std::atomic<std::size_t> num_threads_;   // 表示线程池中的线程数量
	std::queue<Task>         tasks_;         // 任务队列，存储等待执行的任务       
	std::vector<std::thread> pool_;          // 线程池
};