#pragma once

#if __has_include(<experimental/propagate_const>)
	#include <experimental/propagate_const>
	namespace std
	{
		template<typename T>
		using propagate_const = experimental::propagate_const<T>;
	}
#elif __has_include(<propagate_const>)
	#include <propagate_const>
#else

#include <utility>

namespace std {

// incomplete implementation of http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4388.html
template<typename T>
class propagate_const
{
public:
	using element_type = remove_reference_t<decltype(*std::declval<T&>())>;

	constexpr propagate_const() = default;
	propagate_const(const propagate_const&) = delete;
	constexpr propagate_const(propagate_const&&) = default;

	template<typename U>
	constexpr propagate_const(U&& u)
	:
		t_(std::forward<U>(u))
	{
	}

	propagate_const& operator=(const propagate_const&) = delete;
	constexpr propagate_const& operator=(propagate_const&&) = default;

	template<typename U>
	propagate_const& operator=(U&& u)
	{
		t_ = std::forward<U>(u);
		return *this;
	}

	constexpr operator bool() const
	{
		return get() != nullptr;
	}

	constexpr const element_type* get() const
	{
		return t_.get();
	}
	constexpr element_type* get()
	{
		return t_.get();
	}

	constexpr const element_type* operator->() const
	{
		return get();
	}
	constexpr element_type* operator->()
	{
		return get();
	}

private:
	T t_;
};

template<typename T>
bool operator==(const propagate_const<T>& p, nullptr_t)
{
	return p.get() == nullptr;
}
template<typename T>
bool operator!=(const propagate_const<T>& p, nullptr_t)
{
	return p.get() != nullptr;
}

}

#endif

