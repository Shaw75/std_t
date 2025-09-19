#pragma once
// 获取公共类型
template<typename T0, typename T1>
struct common_type_two {
	using type = decltype(0 ? std::declval<T0>() : std::declval<T1>());
};

template<typename ...Ts>
struct common_type {
};

template <typename T0>
struct common_type<T0> {
	using type = T0;
};

template<typename T0, typename T1, typename ...Ts>
struct common_type<T0,T1, Ts...> {
	using type = typename common_type_two<T0, typename common_type<T1, Ts...>::type>::type;
};