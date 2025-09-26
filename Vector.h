#pragma once
template <class T, class Alloc = std::allocator<T>>
struct Vector {

	T* m_data;
	size_t m_size;
	size_t m_cap;
	using allocator = std::allocator<T>;
	[[no_unique_address]] Alloc m_alloc;

	Vector() {
		m_data = nullptr;
		m_size = 0;
		m_cap = 0;
	}
	explicit Vector(size_t n) {
		m_data = allocator{}.allocate(n);
		m_size = n;
		m_cap = n;
		for (size_t i = 0; i < n; i++) {
			std::construct_at(&m_data[i]);
		}

	}
	explicit Vector(size_t n, T const& val) {
		m_data = allocator{}.allocate(n);
		m_size = n;
		m_cap = n;
		for (size_t i = 0; i < n; i++) {
			std::construct_at(&m_data[i], val);
		}
		
	}

	Vector(Vector&& that) noexcept {
		m_data = that.m_data;
		m_size = that.m_size;
		m_cap = that.m_cap;
		that.m_data = nullptr;
		that.m_size = 0;
	}
	Vector& operator=(Vector&& that) noexcept {

		if (m_cap != 0) {
			allocator{}.deallocate(m_data, m_size);
		}
		m_data = that.m_data;
		m_size = that.m_size;
		m_cap = that.m_cap;
		that.m_data = nullptr;
		that.m_size = 0;
		that.m_cap = 0;
		return *this;
	}
	Vector(Vector const& that) {
		m_size = that.m_size;
		m_cap = that.m_size;
		if (m_size != 0) {
			m_data = allocator{}.allocate(m_size);
			for (size_t i = 0; i < m_size; i++) {
				std::construct_at(&m_data[i], std::as_const(that.m_data[i]));
			}
		}
		else {
			m_data = nullptr;
		}
	}
	Vector& operator=(Vector const& that) {
		if (&that == this) [[unlikely]] return *this;
		reserve(that.m_size);
		m_size = that.m_size;
		if (m_size != 0) {
			for (size_t i = 0; i < m_size; i++) {
				std::construct_at(&m_data[i], std::as_const(that.m_data[i]));
			}
		}
		return *this;
	}

	// 定义了析构，一定要定义拷贝和移动，不然会默认生成
	~Vector() noexcept {
		for (size_t i = 0; i != m_size; i++) {
			std::destroy_at(&m_data[i]);
		}
		if (m_cap != 0) {
			m_alloc.deallocate(m_data, m_cap);
		}
		
	}

	template <std::random_access_iterator InputIt>
	Vector(InputIt first, InputIt last) {
		size_t n = last - first;
		m_data = allocator{}.allocate(n);
		m_size = n;
		m_cap = n;
		for (size_t i = 0; i != n; i++) {
			std::construct_at(&m_data[i], *first);
			++first;
		}
	}

	Vector(std::initializer_list<T> ilist) : Vector(ilist.begin(), ilist.end()) {}

	void assgin(size_t n, T const& val) {
		clear();
		reserve(n);
		m_size = n;
		for (size_t i = 0; i < n; i++) {
			std::construct_at(&m_data[i], val);
		}
	}

	void assgin(std::initializer_list<T> ilist) {
		assgin(ilist.begin(), ilist.end());
	}
	template <std::random_access_iterator InputIt>
	void assgin(InputIt first, InputIt last) {
		clear();
		size_t n = last - first;
		reserve(n);
		m_size = n;
		for (size_t i = 0; i < n; i++) {
			std::construct_at(&m_data[i], *first);
			++first;
		}
	}
	T* insert(T const* it, T const&& val) {
		size_t j = it - m_data;
		reserve(m_size + 1);
		for (size_t i = m_size; i != j; i--) {
			std::construct_at(&m_data[i], std::move(m_data[i - 1]));
			std::destroy_at(&m_data[i - 1]);
		}
		m_size += 1;
		std::construct_at(&m_data[j], val);
		return m_data + j;
	}

	T* insert(T const* it, T const& val) {
		size_t j = it - m_data;
		reserve(m_size + 1);
		
		for (size_t i = m_size; i != j; i--) {
			std::construct_at(&m_data[i], std::move(m_data[i - 1]));
			std::destroy_at(&m_data[i - 1]);
		}
		m_size += 1;
		std::construct_at(&m_data[j], val);
		return m_data + j;
	}

	T* insert(T const* it, size_t n, T const& val) {
		size_t j = it - m_data;
		if (n == 0) [[unlikely]] return const_cast<T*>(it);
		reserve(m_size + n);

		for (size_t i = m_size; i != j; i--) {
			std::construct_at(&m_data[n + i - 1], std::move(m_data[i - 1]));
			std::destroy_at(&m_data[i - 1]);
		}
		m_size += n;
		for (size_t i = j; i < j + n; i++) {
			std::construct_at(&m_data[i], val);
		}
		return m_data + j;
	}

	template <std::random_access_iterator InputIt>
	T* insert(T const* it, InputIt first, InputIt last) {
		size_t j = it - m_data;
		size_t n = last - first;
		if (n == 0) [[unlikely]] return const_cast<T*>(it);
		reserve(m_size +n);
		for (size_t i = m_size; i != j; i--) {
			std::construct_at(&m_data[i], std::move(m_data[i - 1]));
			std::destroy_at(&m_data[i - 1]);
		}
		m_size += n;
		for (size_t i = j; i != j + n; i++) {
			std::construct_at(&m_data[i], *first);
			++first;
		}
		return m_data + j;
	}

	void insert(T const* it, std::initializer_list<T> ilist) {
		insert(it, ilist.begin(), ilist.end());
	}


	int const& operator[](size_t i) const {
		return m_data[i];
	}

	int& operator[](size_t i) {
		return m_data[i];
	}
	size_t size() const noexcept {
		return m_size;
	}
	size_t capacity() const noexcept{
		return m_cap;
	}
	void resize(size_t n) {
		reserve(n);
		if (n > m_size)
		{
			for (size_t i = m_size; i < n; i++) {
				std::construct_at(&m_data[i]);
			}
		}
		m_size = n;
	}
	void resize(size_t n, T const& val) {
		reserve(n);
		if (n > m_size)
		{	
			for (size_t i = m_size; i < n; i++) {
				std::construct_at(&m_data[i], val);
			}
		}
		m_size = n;
	}
	void reserve(size_t n) {
		if (n <= m_cap) return;
		n = std::max(n, m_cap * 2);
		auto old_data = m_data;
		auto old_cap = m_cap;

		if (n == 0) {
			m_data = nullptr;
			m_cap = 0;
		}
		else {
			m_data = allocator{}.allocate(n);
			m_cap = n;
		}
		if (old_cap != 0) {
			if (m_size != 0) {
				for (size_t i = 0; i < m_size; i++) {
					std::construct_at(&m_data[i], std::as_const(old_data[i]));
				}
			}
			allocator{}.deallocate(old_data, old_cap);
		}
	}
	void shrink_to_fit() noexcept {
		auto old_data = m_data;
		auto old_cap = m_cap;
		m_cap = m_size;
		if (m_size == 0) {
			m_data = nullptr;
			m_cap = m_size;
		}
		else {
			m_data = allocator{}.allocate(m_size);
		}
		if (old_cap != 0){
			if (m_size != 0) {
				for (size_t i = 0; i < m_size; i++) {
					std::construct_at(&m_data[i], std::as_const(old_data[i]));
				}
			}
			allocator{}.deallocate(old_data, old_cap);
		}
	}
	void clear() {
		for (size_t i = 0; i != m_size; i++) {
			std::destroy_at(&m_data[i]);
		}
		m_size = 0;
	}
	int const& back()const {
		return operator[](size() - 1);
	}
	int& back() {
		return operator[](size() - 1);
	}

	int const& front()const {
		return operator[](0);
	}
	int  front() {
		return operator[](0);
	}
	int& at(size_t i) {
		if (i >= m_size) [[unlikely]] throw std::out_of_range("vector::at");
		return m_data[i];
	}
	int const & at(size_t i) const  {
		if (i >= m_size) [[unlikely]] throw std::out_of_range("vector::at");
		return m_data[i];
	}
	void push_back(T const& val) {
		reserve(m_size + 1);
		std::construct_at(&m_data[m_size], val);
		m_size = m_size + 1;
	}
	void push_back(T const&& val) {
		reserve(m_size + 1);
		std::construct_at(&m_data[m_size],std::move(val));
		m_size = m_size + 1;
	}
	template <class ...Args>
	T& push_back(Args && ...args) {
		reserve(m_size + 1);
		T* p = &m_data[m_size];
		std::construct_at(&m_data[m_size], std::forward<Args>(args)...);
		m_size = m_size + 1;
		return *p;
	}

	T* data() {
		return m_data;
	}
	T const* data()const {
		return m_data;
	}
	T const* cdata()const {
		return m_data;
	}

	T* begin() {
		return m_data;
	}
	T const* begin()const {
		return m_data;
	}
	T const* cbegin()const {
		return m_data;
	}
	T* end() {
		return m_data + m_size;
	}
	T const* end() const {
		return m_data + m_size;
	}
	T const* cend() const {
		return m_data + m_size;
	}
	
	void erase(size_t i) {
		for (size_t j = i + 1; j < m_size; j++) {
			m_data[j - 1] = std::move(m_data[j]);
		}
		resize(m_size - 1);
	}
	void erase(T const* it) {
		size_t i = it - m_data;
		for (size_t j = i + 1; j < m_size; j++) {
			m_data[j - 1] = std::move(m_data[j]);
		}
		resize(m_size - 1);
	}

	void erase(T const* first, T const* last) {
		size_t diff = last - first;
		for (size_t j = last-m_data; j < m_size; j++) {
			m_data[j - diff] = std::move(m_data[j]);
		}
		resize(m_size - diff);
	}


	
};