#pragma once

namespace zls
{
	namespace math
	{
		template<typename T>
		struct traits
		{
			const static T ONE;
			const static T ZERO;
		};

		template<> const float traits<float>::ONE = 1.0f;
		template<> const float traits<float>::ZERO = 0.0f;

		template<> const double traits<double>::ONE = 1.0;
		template<> const double traits<double>::ZERO = 0.0;

	}
}