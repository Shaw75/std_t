#pragma once
#include "Function.hpp"
#include <concepts>

template <typename T>
struct DefaultDeleter {
	void operator()(T* ptr) const {
		delete ptr;
	}
};

template <typename T>
struct DefaultDeleter<T[]> {
	void operator()(T* p) const {
		delete [] p;
	}
};
template<>
struct DefaultDeleter<std::FILE> {
	void operator()(std::FILE* p) const {
		if (p != nullptr) {  // 增加空指针检查更安全
			fclose(p);
		}
	}
};

template <class T,class U>
T exchange(T& dst, U&& val) {
	T tmp = std::move(dst);
	dst = std::forward<U>(val);
	return tmp;
}

template <typename T,typename Deleter = DefaultDeleter<T>>
struct UniquePtr {
public:
	UniquePtr() {};
	explicit UniquePtr(T* ptr)  {
		m_ptr = ptr;
	};
	UniquePtr(const UniquePtr& that) = delete;
	~UniquePtr() {
		if (m_ptr) {
			m_deleter(m_ptr);
		}
		
	}
	template <class U,class UDeleter> 
		requires(std::convertible_to<U*, T*>)
	UniquePtr(UniquePtr<U,UDeleter>&& that) {
		m_ptr = exchange(that.m_ptr, nullptr);
	}
	UniquePtr(UniquePtr&& that)  {
		m_ptr = exchange(that.m_ptr, nullptr);
	}
	UniquePtr(std::nullptr_t = nullptr) {
		m_ptr = nullptr;
	}
	UniquePtr& operator=(const UniquePtr& that) = delete;
	UniquePtr& operator=(UniquePtr&& that) {
		if (this != &that) {
			if (m_ptr) {
				m_deleter(m_ptr);
			}
			m_ptr = exchange(that.m_ptr, nullptr);
		}
	}
	T* get() const {
		return m_ptr;
	}
	T* operator->() const {
		return m_ptr;
	}
	T& operator*() const {
		return *m_ptr;
	}
	T* release() const {
		return exchange(m_ptr,nullptr);
	}
private:
	T* m_ptr;
	Deleter m_deleter;
	template <class U, class UDeleter>
	friend struct UniquePtr;
	
};

template <class T, class Deleter>
struct UniquePtr<T[], Deleter> : UniquePtr<T, Deleter> {
	using UniquePtr<T, Deleter>::UniquePtr;
};

template <class T,class ...Args>
UniquePtr<T> makeUnique(Args&& ...args){
	return UniquePtr<T>(new T(std::forward<Args>(args)...));
}



template <class T>
UniquePtr<T> makeUniqueForOverwrite() {
	return UniquePtr<T>(new T);
}

// 使用 deleter函数实现，特殊的释放
//template <typename T>
//struct UniquePtr {
//public:
//	UniquePtr() {};
//	UniquePtr(T* ptr = nullptr) {
//		m_ptr = ptr;
//		m_deleter = [](T* p) { delete p; };
//	};
//	UniquePtr(Function<void(T*)> deleter, T * ptr = nullptr) {
//		m_ptr = ptr;
//		m_deleter = deleter;
//	};
//	~UniquePtr() {
//		if (m_ptr) {
//			m_deleter(m_ptr);
//		}
//	};
//	UniquePtr(const UniquePtr& that) = delete;
//	UniquePtr(UniquePtr&& that) {
//		m_ptr = that.m_ptr;
//		that.m_ptr = nullptr;
//	}
//	UniquePtr& operator=(const UniquePtr& that) = delete;
//	UniquePtr& operator=(UniquePtr&& that) {
//		if (this != that) {
//			m_ptr = that.m_ptr;
//			that.m_ptr = nullptr;
//		}
//	}
//private:
//	T* m_ptr{ nullptr };
//	Function<void(T*)> m_deleter;
//};