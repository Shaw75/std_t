#pragma once
template <class T, size_t N>
struct Array {
public:
	using value_type = T;
	using iterator = T*;
	using const_iterator = T const *;
	T m_elements[N];
	T& operator[](size_t i) noexcept  {
		return m_elements[i];
	}
	T const& operator[](size_t i) const noexcept {
		return m_elements[i];
	}
	T& at(size_t i) {
		if (i >= N) [[unlikely]] throw std::runtime_error("out of range!");
		return m_elements[i];
	}

	T const& at(size_t i) const  {
		if (i >= N) [[unlikely]] throw std::runtime_error("out of range!");
		return m_elements[i];
	}
	static constexpr size_t size()  noexcept {
		return N;
	}
	T* begin() noexcept {
		return m_elements;
	}
	T* end() noexcept {
		return m_elements + N;
	}
	T const* begin() const  noexcept {
		return m_elements;
	}
	T const* end() const noexcept  {
		return m_elements + N;
	}
	T const* cbegin() const  noexcept {
		return m_elements;
	}
	T const* cend() const noexcept {
		return m_elements + N;
	}
	T* data() noexcept {
		return m_elements;
	}
	T const* data()  const noexcept {   // 重载通过推导使用const版本
		return m_elements;
	}
	T  const* cdata()  const noexcept {// 强制使用const版本
		return m_elements;
	}

	T& front() noexcept {
		return m_elements[0];
	}
	T const& front() const noexcept {
		return m_elements[0];
	}
	T& back() noexcept {
		return m_elements[N-1];
	}
	T const& back() const noexcept {
		return m_elements[N-1];
	}
	void fill(T const& val) {
		for (size_t i = 0; i < N; i++) {
			m_elements[i] = val;
		}
	}
	void swap(Array& that) {
		for (size_t i = 0; i < N; i++) {
			std::swap(m_elements[i], that.m_elements[i]);
		}
	}
};

template <class T, class ...Ts>
Array(T, Ts...) -> Array<T, 1 + sizeof...(Ts)>;
