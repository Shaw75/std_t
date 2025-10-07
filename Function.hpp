#pragma once
/*
*  µœ÷std::function
*/
#include <memory>
#include <stdexcept>

template <class FnSig>
struct Function {
	static_assert(!std::is_same_v<FnSig, FnSig>, "not a valid function signature");
};
// void(int)
template <typename Ret, typename ...Args>
struct Function<Ret(Args...)> {
	
	 struct FuncBase {
		virtual Ret call(Args ...args) = 0;
		virtual ~FuncBase() = default;
	 };
	
	template<typename F>
	struct FuncImpl : FuncBase{
		F f;
		FuncImpl(F f) : f{ std::move(f)} {};
		virtual Ret call(Args ...args) override {
			return std::invoke(f, std::forward<Args>(args)...);
		}
	};

	Function() = default;
	template <typename F>
		requires (std::is_invocable_r_v<Ret, F, Args...>
		&& !std::is_same_v<std::decay_t<F>, Function>)
	Function(F f) : ptr_{ std::make_shared<FuncImpl<F>>(std::move(f)) } {};
	Ret operator()(Args ...args) const {
		if (!ptr_) throw std::runtime_error("function uninitialized");
		return ptr_->call(std::forward<Args>(args)...);
	}
	std::shared_ptr<FuncBase> ptr_;
};