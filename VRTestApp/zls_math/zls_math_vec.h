#pragma once

namespace zls {
	namespace math {

		template<typename T>
		class Vec2
		{
		private:
			union {
				T x; T r; T s;
			};
			union {
				T y; T g; T t;
			};
		public:
			Vec2() : x(zls::math::traits<T>::ZERO), y(zls::math::traits<T>::ZERO)
			{
			}
			Vec2(const T _x, const T _y) : x(_x), y(_y)
			{
			}
		};

		template<typename T>
		class Vec3
		{
		public:
			union {
				T x; T r; T s;
			};
			union {
				T y; T g; T t;
			};
			union {
				T z; T b; T p;
			};
		public:
			Vec3() : x(zls::math::traits<T>::ZERO), y(zls::math::traits<T>::ZERO), z(zls::math::traits<T>::ZERO)
			{
			}
			Vec3(const T _x, const T _y, const T _z) : x(_x), y(_y), z(_z)
			{
			}
		};

		template<typename T>
		class Vec4
		{
		private:
			union {
				T x; T r; T s;
			};
			union {
				T y; T g; T t;
			};
			union {
				T z; T b; T p;
			};
			union {
				T w; T a; T q;
			};
		public:
			Vec4() : x(zls::math::traits<T>::ZERO), y(zls::math::traits<T>::ZERO), z(zls::math::traits<T>::ZERO), w(zls::math::traits<T>::ONE)
			{
			}
			Vec4(const T _x, const T _y, const T _z, const T _w) : x(_x), y(_y), z(_z), w(_w)
			{
			}
		};

		typedef Vec2<float> vec2;
		typedef Vec3<float> vec3;
		typedef Vec4<float> vec4;

		typedef Vec2<double> dvec2;
		typedef Vec3<double> dvec3;
		typedef Vec4<double> dvec4;

		typedef Vec2<bool> bvec2;
		typedef Vec3<bool> bvec3;
		typedef Vec4<bool> bvec4;

		typedef Vec2<int> ivec2;
		typedef Vec3<int> ivec3;
		typedef Vec4<int> ivec4;

		typedef Vec2<unsigned int> uvec2;
		typedef Vec3<unsigned int> uvec3;
		typedef Vec4<unsigned int> uvec4;
	}
}