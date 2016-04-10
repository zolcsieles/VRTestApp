#pragma once

namespace zls
{
	namespace math
	{
		template<typename T>
		T cos(const T _rad)
		{
			return ::cos(_rad);
		}

		template<typename T>
		T sin(const T _rad)
		{
			return ::sin(_rad);
		}

		template<typename T>
		T sqrt(const T _sq)
		{
			return ::sqrt(_sq);
		}
	}
}
