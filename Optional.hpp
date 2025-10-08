#pragma once
#include <exception>

struct BadOptionalAccess : public std::exception {
	BadOptionalAccess() = default;
	virtual ~BadOptionalAccess() = default;
	const char* what() const noexcept override {
		return "bad optional access!";
	}
};

struct nullopt_t {
	explicit nullopt_t() = default;
};

struct InPlace {
	explicit InPlace() = default;
};

constexpr InPlace inPlace;

inline constexpr nullopt_t nullopt;

template <class T>
struct Optional {
private:
	bool m_has_value;
	union {
		T m_value;
		nullopt_t m_nullopt;
	};
	
public:
	Optional(T&& value) noexcept : m_has_value(true), m_value(std::move(value)) {}

	Optional(T const& value) noexcept : m_has_value(true), m_value(std::move(value)) {}

	Optional() noexcept : m_has_value(false) {}

	Optional(nullopt_t) noexcept : m_has_value(false) {}

	template <class ...Ts>
	explicit Optional(InPlace, Ts &&...value_args) : m_has_value(true), m_value(std::forward<Ts>(value_args)...) {}

	template <class U, class ...Ts>
	explicit Optional(InPlace, std::initializer_list<U> ilist, Ts &&...value_args) : m_has_value(true),
		m_value(ilist, std::forward<Ts>(value_args)...) {
	}

	Optional(const Optional& that) : m_has_value(that.m_has_value){
		if (m_has_value) {
			new(&m_value) T(that.m_value);
		}
	}
	Optional(Optional&& that) noexcept  : m_has_value(that.m_has_value)  {
		if (m_has_value) {
			new(&m_value) T(std::move(that.m_value));
		}
	}
	Optional& operator=(nullopt_t) noexcept  {
		if (m_has_value) {
			m_value.~T();
			m_has_value = false;
		}
		return *this;
	}

	Optional& operator=(const T& value) noexcept {
		if (m_has_value) {
			m_value.~T();
			m_has_value = false;
		}
		m_has_value = true;
		new(&m_value) T(value);
		return *this;
	}

	Optional& operator=(T&& value) noexcept  {
		if (m_has_value) {
			m_value.~T();
			m_has_value = false;
		}
		m_has_value = true;
		new(&m_value) T(std::move(value));
		return *this;
	}

	Optional& operator=(const Optional& that) {
		if (m_has_value) {
			m_value.~T();
			m_has_value = false;
		}
		if (that.m_has_value) {
			new(&m_value) T(that.value);
		}
		m_has_value = that.m_has_value;
		return *this;
	}
	Optional& operator=(Optional&& that) noexcept {
		if (m_has_value) {
			m_value.~T();
			m_has_value = false;
		}
		if (that.m_has_value) {
			new(&m_value) T(std::move(that.value));
			that.m_value.~T();
		}
		m_has_value = that.m_has_value;
		that.m_has_value = false;
		return *this;
	}
	~Optional() noexcept {
		if (m_has_value) {
			m_value.~T();
		}
	}
	bool has_value() const noexcept {
		return has_value;
	}
	T& value() & {
		if (!m_has_value) throw BadOptionalAccess();
		return m_value;
	}
	const T&  value() const& {
		if (!m_has_value) throw BadOptionalAccess();
		return m_value;
	}

	T&& value()&& {
		if (!m_has_value) throw BadOptionalAccess();
		return std::move(m_value);
	}
	const T&& value() const&& {
		if (!m_has_value) throw BadOptionalAccess();
		return std::move(m_value);
	}

	explicit operator bool() const noexcept {
		return m_has_value;
	}

	T& operator*()& noexcept{
		return m_value;
	}
	const T& operator*() const& noexcept {
		return m_value;
	}

	T&& operator*() && noexcept {
		return std::move(m_value);
	}

	const T&& operator*() const&& noexcept {
		return std::move(m_value);
	}

	const T* operator->() const noexcept {
		return &m_value;
	}
	T* operator->() noexcept {
		return &m_value;
	}

	bool operator==(nullopt_t) const noexcept {
		return !m_has_value;
	}

	friend bool operator==(nullopt_t, Optional const& self) noexcept {
		return !self.m_has_value;
	}

	bool operator==(Optional<T> const& that) const noexcept {
		if (m_has_value != that.m_has_value)
			return false;
		if (m_has_value) {
			return m_value == that.m_value;
		}
		return true;
	}

	bool operator!=(Optional const& that) const noexcept {
		if (m_has_value != that.m_has_value)
			return true;
		if (m_has_value) {
			return m_value != that.m_value;
		}
		return false;
	}

	bool operator>(Optional const& that) const noexcept {
		if (!m_has_value || !that.m_has_value)
			return false;
		return m_value > that.m_value;
	}
	bool operator<(Optional const& that) const noexcept {
		if (!m_has_value || !that.m_has_value)
			return false;
		return m_value < that.m_value;
	}

	bool operator>=(Optional const& that) const noexcept {
		if (!m_has_value || !that.m_has_value)
			return true;
		return m_value >= that.m_value;
	}

	bool operator<=(Optional const& that) const noexcept {
		if (!m_has_value || !that.m_has_value)
			return true;
		return m_value <= that.m_value;
	}
	T value_or(T default_value) const& noexcept {
		if (!m_has_value)
			return default_value;
		return m_value;
	}
	T value_or(T default_value) && noexcept {
		if (!m_has_value)
			return default_value;
		return std::move(m_value);
	}

	template <class ...Ts>
	void emplace(Ts&& ...value_args) {
		if (m_has_value) {
			m_value.~T();
			m_has_value = false;
		}
		new(&m_value) T(std::forward<Ts>(value_args)...);
		m_has_value = true;
	}
	void reset() noexcept {
		if (m_has_value) {
			m_value.~T(); 
			m_has_value = false;
		}
		return *this;
	}
	template <class F>
	auto and_then(F&& f) const&
		-> typename std::remove_cv<
		typename std::remove_reference<
		decltype(f(m_value))>::type>::type {
		if (m_has_value) {
			return std::forward<F>(f)(m_value);
		}
		else {
			return typename std::remove_cv<
				typename std::remove_reference<
				decltype(f(m_value))>::type>::type{};
		}
	}

	template <class F>
	auto and_then(F&& f) &
		-> typename std::remove_cv<
		typename std::remove_reference<
		decltype(f(m_value))>::type>::type {
		if (m_has_value) {
			return std::forward<F>(f)(m_value);
		}
		else {
			return typename std::remove_cv<
				typename std::remove_reference<
				decltype(f(m_value))>::type>::type{};
		}
	}

	template <class F>
	auto and_then(F&& f) const&&
		-> typename std::remove_cv<
		typename std::remove_reference<
		decltype(f(std::move(m_value)))>::type>::type {
		if (m_has_value) {
			return std::forward<F>(f)(std::move(m_value));
		}
		else {
			return typename std::remove_cv<
				typename std::remove_reference<
				decltype(f(std::move(m_value)))>::type>::type{};
		}
	}

	template <class F>
	auto and_then(F&& f) &&
		-> typename std::remove_cv<
		typename std::remove_reference<
		decltype(f(std::move(m_value)))>::type>::type {
		if (m_has_value) {
			return std::forward<F>(f)(std::move(m_value));
		}
		else {
			return typename std::remove_cv<
				typename std::remove_reference<
				decltype(f(std::move(m_value)))>::type>::type{};
		}
	}
	template <class F>
	auto transform(F&& f) const&
		-> Optional<typename std::remove_cv<
		typename std::remove_reference<
		decltype(f(m_value))>::type>::type> {
		if (m_has_value) {
			return std::forward<F>(f)(m_value);
		}
		else {
			return nullopt;
		}
	}

	template <class F>
	auto transform(F&& f) &
		-> Optional<typename std::remove_cv<
		typename std::remove_reference<
		decltype(f(m_value))>::type>::type> {
		if (m_has_value) {
			return std::forward<F>(f)(m_value);
		}
		else {
			return nullopt;
		}
	}

	template <class F>
	auto transform(F&& f) const&&
		-> Optional<typename std::remove_cv<
		typename std::remove_reference<
		decltype(f(std::move(m_value)))>::type>::type> {
		if (m_has_value) {
			return std::forward<F>(f)(std::move(m_value));
		}
		else {
			return nullopt;
		}
	}

	template <class F>
	auto transform(F&& f) &&
		-> Optional<typename std::remove_cv<
		typename std::remove_reference<
		decltype(f(std::move(m_value)))>::type>::type> {
		if (m_has_value) {
			return std::forward<F>(f)(std::move(m_value));
		}
		else {
			return nullopt;
		}
	}

	template <class F, typename std::enable_if<std::is_copy_constructible<T>::value, int>::type = 0>
	Optional or_else(F&& f) const& {
		if (m_has_value) {
			return *this;
		}
		else {
			return std::forward<F>(f)();
		}
	}

	template <class F, typename std::enable_if<std::is_move_constructible<T>::value, int>::type = 0>
	Optional or_else(F&& f) && {
		if (m_has_value) {
			return std::move(*this);
		}
		else {
			return std::forward<F>(f)();
		}
	}

	void swap(Optional& that) noexcept {
		if (m_has_value && that.m_has_value) {
			using std::swap; // ADL  //
			swap(m_value, that.m_value);
		}
		else if (!m_has_value && !that.m_has_value) {
			// do nothing
		}
		else if (m_has_value) {
			that.emplace(std::move(m_value));
			reset();
		}
		else {
			emplace(std::move(that.m_value));
			that.reset();
		}
	}
};


#if __cpp_deduction_guides
template <class T> // C++17 ╡есп CTAD
Optional(T) -> Optional<T>;
#endif

template <class T>
Optional<T> makeOptional(T value) {
	return Optional<T>(std::move(value));
}