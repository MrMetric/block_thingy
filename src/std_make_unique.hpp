#pragma once
// source: http://stackoverflow.com/a/17902439/1578318

#include <memory>

#if !defined(_MSC_VER) // dunno why __cpp_lib_make_unique is not defined
#if __cpp_lib_make_unique < 201304

#include <cstddef>
#include <type_traits>
#include <utility>

namespace std
{
	template<class T> struct _Unique_if
	{
		typedef unique_ptr<T> _Single_object;
	};

	template<class T> struct _Unique_if<T[]>
	{
		typedef unique_ptr<T[]> _Unknown_bound;
	};

	template<class T, size_t N> struct _Unique_if<T[N]>
	{
		typedef void _Known_bound;
	};

	template<class T, class... Args>
		typename _Unique_if<T>::_Single_object
		make_unique(Args&&... args)
		{
			return unique_ptr<T>(new T(std::forward<Args>(args)...));
		}

	template<class T>
		typename _Unique_if<T>::_Unknown_bound
		make_unique(size_t n)
		{
			typedef typename remove_extent<T>::type U;
			return unique_ptr<T>(new U[n]());
		}

	template<class T, class... Args>
		typename _Unique_if<T>::_Known_bound
		make_unique(Args&&...) = delete;
}

#define __cpp_lib_make_unique 201304

#endif // __cpp_lib_make_unique < 201304
#endif // !defined(_LIBCPP_STD_VER) && !defined(_MSC_VER)
