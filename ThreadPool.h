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
	num_threads = num_threads == 0 ? 2 : num_threads; // ��ֹ�޷���⵱ǰӲ������֤�̳߳������������߳�
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
		// start����
		start();
	}
	~ThreadPool() {
		stop();
	}

	// �������������������
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
			tasks_.emplace([task] {(*task)(); }); // ����ָ������*���ض�Ӧ�������� ��packaged_task���أ�����ʾ����
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
	std::mutex               mutex_;         // ������
	std::condition_variable  cv_;            // ��������
	std::atomic<bool>        stop_;          // ָʾ�̳߳��Ƿ�ֹͣ   
	std::atomic<std::size_t> num_threads_;   // ��ʾ�̳߳��е��߳�����
	std::queue<Task>         tasks_;         // ������У��洢�ȴ�ִ�е�����       
	std::vector<std::thread> pool_;          // �̳߳�
};