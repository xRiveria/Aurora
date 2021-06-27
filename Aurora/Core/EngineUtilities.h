#pragma once
#include <functional>

namespace Aurora::Utilities
{
	template <class T>
	inline constexpr void Hash_Combine(std::size_t& seed, const T& v)
	{
		std::hash<T> hasher;
		seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}
}