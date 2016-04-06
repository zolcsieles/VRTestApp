#pragma once

namespace zls
{
	namespace math
	{
		template<typename T, int row, int col>
		class _math
		{
		protected:
			T m[row][col];
		};

		template<typename T>
		class Mat2 : public _math<T, 2, 2>
		{
		public:
			Mat2()
			{
				SetIdentity();
			}
			Mat2(
				const T _m00, const T _m01,
				const T _m10, const T _m11
				)
			{
				m[0][0] = _m00;
				m[0][1] = _m01;

				m[1][0] = _m10;
				m[1][1] = _m11;
			}

			//

			void SetIdentity()
			{
				          m[0][1] = 
				m[1][0] =           zls::math::traits<T>::ZERO;
				m[0][0] = m[1][1] = zls::math::traits<T>::ONE;
			}
		};

		template<typename T>
		class Mat3 : public _math<T, 3, 3>
		{
		public:
			Mat3()
			{
				SetIdentity();
			}
			Mat3(
				const T _m00, const T _m01, const T _m02,
				const T _m10, const T _m11, const T _m12,
				const T _m20, const T _m21, const T _m22
				)
			{
				m[0][0] = _m00;
				m[0][1] = _m01;
				m[0][2] = _m02;

				m[1][0] = _m10;
				m[1][1] = _m11;
				m[1][2] = _m12;

				m[2][0] = _m20;
				m[2][1] = _m21;
				m[2][2] = _m22;
			}

			//

			void SetIdentity()
			{
				          m[0][1] = m[0][2] =
				m[1][0] =           m[1][2] =
				m[2][0] = m[2][1] =           zls::math::traits<T>::ZERO;
				m[0][0] = [1][1] =  m[2][2] = zls::math::traits<T>::ONE;
			}
		};

		template<typename T>
		class Mat4 : public _math<T, 4, 4>
		{
		public:
			Mat4()
			{
				SetIdentity();
			}
			Mat4(
				const T _m00, const T _m01, const T _m02, const T _m03,
				const T _m10, const T _m11, const T _m12, const T _m13,
				const T _m20, const T _m21, const T _m22, const T _m23,
				const T _m30, const T _m31, const T _m32, const T _m33
				)
			{
				m[0][0] = _m00;
				m[0][1] = _m01;
				m[0][2] = _m02;
				m[0][3] = _m03;

				m[1][0] = _m10;
				m[1][1] = _m11;
				m[1][2] = _m12;
				m[1][3] = _m13;

				m[2][0] = _m20;
				m[2][1] = _m21;
				m[2][2] = _m22;
				m[2][3] = _m23;

				m[3][0] = _m30;
				m[3][1] = _m31;
				m[3][2] = _m32;
				m[3][3] = _m33;
			}

			//

			void SetIdentity()
			{
				          m[0][1] = m[0][2] = m[0][3] =
				m[1][0] =           m[1][2] = m[1][3] =
				m[2][0] = m[2][1] =           m[2][3] = 
				m[3][0] = m[3][1] = m[3][2]           = zls::math::traits<T>::ZERO;
				m[0][0] = m[1][1] = m[2][2] = m[3][3] = zls::math::traits<T>::ONE;
			}

			void SetTranslate(T x, T y, T z)
			{
				m[0][0] = zls::math::traits<T>::ONE;
				m[0][1] = zls::math::traits<T>::ZERO;
				m[0][2] = zls::math::traits<T>::ZERO;
				m[0][3] = zls::math::traits<T>::ZERO;

				m[1][0] = zls::math::traits<T>::ZERO;
				m[1][1] = zls::math::traits<T>::ONE;
				m[1][2] = zls::math::traits<T>::ZERO;
				m[1][3] = zls::math::traits<T>::ZERO;

				m[2][0] = zls::math::traits<T>::ZERO;
				m[2][1] = zls::math::traits<T>::ZERO;
				m[2][2] = zls::math::traits<T>::ONE;
				m[2][3] = zls::math::traits<T>::ZERO;

				m[3][0] = x;
				m[3][1] = y;
				m[3][2] = z;
				m[3][3] = zls::math::traits<T>::ONE;
			}

			void SetFrustum(const T _near, const T _far, const T _top, const T _bottom, const T _left, const T _right)
			{
				const T n2 = 2 * _near;
				const T rpl = _right + _left;
				const T rml = _right - _left;
				const T tmb = _top - _bottom;
				const T tpb = _top + _bottom;
				const T fpn = _far + _near;
				const T fmn = _far - _near;

				m[0][0] = n2 / rml;
				m[1][0] = 0.0;
				m[2][0] = rpl / rml;
				m[3][0] = 0.0;

				m[0][1] = 0.0;
				m[1][1] = n2 / tmb;
				m[2][1] = tpb / tmb;
				m[3][1] = 0.0;

				m[0][2] = 0.0;
				m[1][2] = 0.0;
				m[2][2] = (-fpn) / fmn;
				m[3][2] = (-n2*_far) / fmn;

				m[0][3] = 0.0;
				m[1][3] = 0.0;
				m[2][3] = -1.0;
				m[3][3] = 0.0;
			}

			void SetSymetricFrustum(const T _near, const T _far, T _top, T _right)
			{
				const T fmn = _far - _near;

				m[0][0] = _near / _right;
				m[1][0] = 0.0;
				m[2][0] = 0.0;
				m[3][0] = 0.0;

				m[0][1] = 0.0;
				m[1][1] = _near / _top;
				m[2][1] = 0.0;
				m[3][1] = 0.0;

				m[0][2] = 0.0;
				m[1][2] = 0.0;
				m[2][2] = -(_far+_near) / fmn;
				m[3][2] = (-2*_far*_near) / fmn;

				m[0][3] = 0.0;
				m[1][3] = 0.0;
				m[2][3] = -1.0;
				m[3][3] = 0.0;
			}

			void SetRotateX(T degree)
			{
				const T rad = T(degree * M_PI / 180.0);
				m[0][0] = 1.0f;
				m[1][1] = zls::math::cos(rad);
				m[2][2] = m[1][1];
				m[3][3] = 1.0f;

				m[1][2] = zls::math::sin(rad);
				m[2][1] = -m[1][2];
			}

			//

			T* operator ()()
			{
				return &m[0][0];
			}
		};

		typedef Mat2<float> mat2;
		typedef Mat3<float> mat3;
		typedef Mat4<float> mat4;

		typedef Mat2<float> mat2x2;
		typedef Mat3<float> mat3x3;
		typedef Mat4<float> mat4x4;

		typedef Mat2<double> dmat2;
		typedef Mat3<double> dmat3;
		typedef Mat4<double> dmat4;

		typedef Mat2<double> dmat2x2;
		typedef Mat3<double> dmat3x3;
		typedef Mat4<double> dmat4x4;
	}
}

#if DISABLED && !DISABLED
mat2, mat3, mat4						2x2, 3x3, 4x4 float matrix
mat2x2, mat2x3, mat2x4					2 - column float matrix of 2, 3, or 4 rows
mat3x2, mat3x3, mat3x4					3 - column float matrix of 2, 3, or 4 rows
mat4x2, mat4x3, mat4x4					4 - column float matrix of 2, 3, or 4 rows
dmat2, dmat3, dmat4						2x2, 3x3, 4x4 double - precision float matrix 
dmat2x2, dmat2x3, dmat2x4				2 - col. double - precision float matrix of 2, 3, 4 rows
dmat3x2, dmat3x3, dmat3x4				3 - col. double - precision float matrix of 2, 3, 4 rows 
dmat4x2, dmat4x3, dmat4x4				4 - column double - precision float matrix of 2, 3, 4 rows
#endif
