#ifndef __TYPE_TRAITS__
#define __TYPE_TRAITS__

#if __cplusplus >= 201103L
#	include <type_traits>
#else
namespace std {
	// Simula o mesmo tipo da `std` (C++11).
	template <typename T, T v>
	struct integral_constant {
		typedef T value_type;
		static const T value = v;

		operator value_type() const {
			return value;
		}
	};

	// Simula o mesmo tipo da `std` (C++11).
	typedef integral_constant<bool, true> true_type;
	// Simula o mesmo tipo da `std` (C++11).
	typedef integral_constant<bool, false> false_type;

	// Simula o mesmo tipo da `std` (C++11).
	template <typename T, typename U>
	struct is_same: false_type {};
	// Simula o mesmo tipo da `std` (C++11).
	template <typename T>
	struct is_same<T, T>: true_type {};
}
#endif

#endif//__TYPE_TRAITS__
