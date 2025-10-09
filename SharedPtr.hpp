#pragma once
#include "UniquePtr.hpp"
#include <algorithm>
#include <atomic>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>


struct _SpcontrolBlock {
	std::atomic<long> m_refcnt;
	explicit _SpcontrolBlock() :  m_refcnt{ 1 } {}
	explicit _SpcontrolBlock(_SpcontrolBlock&&) = delete;
	void _M_incRef() noexcept {
		m_refcnt.fetch_add(1, std::memory_order_relaxed);

	}
	void _M_decRef() noexcept  {
		if (m_refcnt.fetch_sub(1, std::memory_order_relaxed) == 1) {
			delete this;
		}
	}
	virtual ~_SpcontrolBlock() = default;
};

template <class T, class Deleter>
struct _SpcontrolBlockImpl : _SpcontrolBlock {
	T* m_ptr;
	[[no_unique_address]]  Deleter m_deleter;
	explicit _SpcontrolBlockImpl(T* ptr) noexcept : m_ptr{ ptr }{}
	explicit _SpcontrolBlockImpl(T* ptr, Deleter deleter) noexcept : m_ptr{ ptr }, m_deleter{std::move(deleter)} {}
	~_SpcontrolBlockImpl() override {
		m_deleter(m_ptr);
	}
};

template <class T>
struct SharedPtr {
public:
	using element_type = T;
	using pointer = T*;
	SharedPtr(std::nullptr_t = nullptr) noexcept : m_cb{ nullptr } {}

	template <class Y,
		std::enable_if_t<std::is_convertible_v<Y*, T*>, int> = 0>
	explicit SharedPtr(Y* ptr)  
		: m_ptr{ ptr }, m_cb{ new _SpcontrolBlockImpl<Y, DefaultDeleter<Y>>(ptr) } {}

	template <class Y, class Deleter,
		std::enable_if_t<std::is_convertible_v<Y*, T*>, int> = 0>
	explicit SharedPtr(Y* ptr, Deleter deleter)  
		: m_ptr{ ptr }, m_cb{ new _SpcontrolBlockImpl<Y, Deleter>(ptr, std::move(deleter)) } {}

	SharedPtr(const SharedPtr& that) noexcept : m_ptr{ that.m_ptr }, m_cb{that.m_cb} {
		if (m_cb) {
			m_cb->_M_incRef();
		}
	}

	template <class Y,
		std::enable_if_t<std::is_convertible_v<Y*, T*>, int> = 0>
	SharedPtr(SharedPtr<Y> const& that) noexcept : m_ptr{ that.m_ptrptr }, m_cb{ that.m_cb } {
		if (m_cb) {
			m_cb->_M_incRef();
		}
	}

	SharedPtr(SharedPtr&& that) noexcept : m_ptr{ that.m_ptr }, m_cb{ that.m_cb } {
		that.m_cb = nullptr;
		that.m_ptr = nullptr;
	}

	template <class Y,
		std::enable_if_t<std::is_convertible_v<Y*, T*>, int> = 0>
	SharedPtr(SharedPtr<Y>&& that) noexcept : m_ptr(that.m_ptr), m_cb(that.m_cb) {
		that.m_cb = nullptr;
		that.m_ptr = nullptr;
	}


	template <class Y>
	SharedPtr(const SharedPtr<Y>& that, T* ptr) noexcept : m_ptr{ ptr }, m_cb{ that.m_cb } {
		if (m_cb) {
			m_cb->_M_incRef();
		}
	}

	template <class Y>
	SharedPtr(SharedPtr<Y>&& that, T* ptr) noexcept : m_ptr{ ptr }, m_cb{ that.m_cb } {
		that.m_cb = nullptr;
		that.m_ptr = nullptr;
	}

	SharedPtr& operator=(const SharedPtr& that) noexcept {
		if (this == &that) {
			return *this;
		}
		m_ptr = that.m_ptr;
		m_cb = that.m_cb;
		that.m_cb = nullptr;
		that.m_ptr = nullptr;
		if (m_cb) {
			m_cb->_M_decRef();
		}
		return *this;
	}

	SharedPtr& operator=(SharedPtr&& that) noexcept {
		if (this == &that) {
			return *this;
		}
		if (m_cb) {
			m_cb->_M_decRef();
		}
		m_ptr = that.m_ptr;
		m_cb = that.m_cb;
		that.m_ptr = nullptr;
		that.m_cb = nullptr;
		return *this;
	}
	
	template <class Y,
		std::enable_if_t<std::is_convertible_v<Y*, T*>, int> = 0>
	SharedPtr& operator=(SharedPtr<Y> const& that) noexcept {
		if (this == &that) {
			return *this;
		}
		if (m_cb) {
			m_cb->_M_decRef();
		}
		m_ptr = that.m_ptr;
		m_cb =  that.m_cb;
		if (m_cb) {
			m_cb->_M_incRef();
		}
		return *this;
	}

	template <class Y,
		std::enable_if_t<std::is_convertible_v<Y*, T*>, int> = 0>
	SharedPtr& operator=(SharedPtr<Y>&& that) noexcept {
		if (this == &that) {
			return *this;
		}
		if (m_cb) {
			m_cb->_M_decRef();
		}
		m_ptr = that.m_ptr;
		m_cb = that.m_cb;
		that.m_ptr = nullptr;
		that.m_cb = nullptr;
		return *this;
	}

	~SharedPtr() noexcept {
		if(m_cb)
			m_cb->_M_decRef();
	}

	

	void reset() noexcept {
		m_cb->_M_decRef();
		m_cb = nullptr;
		m_ptr = nullptr;
	}

	template <class Y>
	void reset(Y* ptr)  {
		m_cb->_M_decRef();
		m_cb = nullptr;
		m_ptr = nullptr;
		m_ptr = ptr;
		m_cb = new _SpcontrolBlockImpl < Y, DefaultDeleter<Y>>(ptr);
	}

	template <class Y, class Deleter>
	void reset(Y* ptr,Deleter deleter) {
		m_cb->_M_decRef();
		m_cb = nullptr;
		m_ptr = nullptr;
		m_ptr = ptr;
		m_cb = new _SpcontrolBlockImpl <Y, Deleter>(ptr, std::move(deleter));
	}

	long use_count() noexcept {
		return m_cb ? m_cb->m_refcnt() : 0;
	}
	bool unique() noexcept {
		return m_cb ? m_cb->m_refcnt() == 1 : true;
	}
	template <class Y>
	bool operator<(const SharedPtr<Y>& that) noexcept {
		return m_ptr == that.m_ptr;
	}
	template <class Y>
	bool operator==(const SharedPtr<Y>& that) noexcept {
		return m_ptr < that.m_ptr;
	}
	template <class Y>
	bool owner_equal(const SharedPtr<Y>& that) noexcept {
		return m_cb == that.m_cb;
	}
	template <class Y>
	bool owner_before(const SharedPtr<Y>& that) noexcept {
		return m_cb < that.m_cb;
	}
	void swap(SharedPtr& that) noexcept {
		std::swap(m_ptr, that.m_ptr);
		std::swap(m_cb, that.m_cb);
	}

	T* get() const noexcept {
		return m_ptr;
	}
	T* operator->() const noexcept {
		return m_ptr;
	}

private:
	T* m_ptr;
	_SpcontrolBlock* m_cb;
};

template <class T>
struct SharedPtr<T[]> : SharedPtr<T> {
	using SharedPtr<T>::SharedPtr;

	std::add_lvalue_reference_t<T> operator[](std::size_t __i) {
		return this->get()[__i];
	}
};


template <class T, class ...Args,
	std::enable_if_t<!std::is_array_v<T>, int> = 0>
SharedPtr<T> makeShared(Args&& ...args) {
	return SharedPtr<T>(new T(std::forward<Args>(args)...));
}

template <class T, class... _Args,
	std::enable_if_t<std::is_unbounded_array_v<T>, int> = 0>
SharedPtr<T> makeShared(std::size_t __len) {
	std::remove_extent_t<T>* __p = new std::remove_extent_t<T>[__len];
	try {
		return SharedPtr<T>(__p);
	}
	catch (...) {
		delete[] __p;
		throw;
	}
}


