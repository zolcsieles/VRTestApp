#pragma once

namespace zls
{
	namespace math
	{
		template<typename T, int col, int row>
		class _math
		{
		protected:
			//m[col][row]
			T m[col][row];

			inline void CopyTo(_math* target) const
			{
				memcpy(target->m, m, sizeof(m));
			}
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
				m[0][1] = _m10;

				m[1][0] = _m01;
				m[1][1] = _m11;
			}

			void SetIdentity()
			{
				          m[1][0] = 
				m[0][1] =           zls::math::traits<T>::ZERO;
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
				m[0][1] = _m10;
				m[0][2] = _m20;

				m[1][0] = _m01;
				m[1][1] = _m11;
				m[1][2] = _m21;

				m[2][0] = _m02;
				m[2][1] = _m12;
				m[2][2] = _m22;
			}

			void SetIdentity()
			{
				          m[1][0] = m[2][0] =
				m[0][1] =           m[2][1] =
				m[0][2] = m[1][2] =           zls::math::traits<T>::ZERO;
				m[0][0] = m[1][1] = m[2][2] = zls::math::traits<T>::ONE;
			}
		};

		template<typename T>
		class Mat4 : public _math<T, 4, 4>
		{
		private:
			static const Mat4 identity;
		public:
			Mat4()
			{
			}
			Mat4(
				const T _m00, const T _m01, const T _m02, const T _m03,
				const T _m10, const T _m11, const T _m12, const T _m13,
				const T _m20, const T _m21, const T _m22, const T _m23,
				const T _m30, const T _m31, const T _m32, const T _m33
				)
			{
				m[0][0] = _m00;
				m[0][1] = _m10;
				m[0][2] = _m20;
				m[0][3] = _m30;

				m[1][0] = _m01;
				m[1][1] = _m11;
				m[1][2] = _m21;
				m[1][3] = _m31;

				m[2][0] = _m02;
				m[2][1] = _m12;
				m[2][2] = _m22;
				m[2][3] = _m32;

				m[3][0] = _m03;
				m[3][1] = _m13;
				m[3][2] = _m23;
				m[3][3] = _m33;
			}

			//
			void SetIdentity()
			{
				          m[1][0] = m[2][0] = m[3][0] =
				m[0][1] =           m[2][1] = m[3][1] =
				m[0][2] = m[1][2] =           m[3][2] = 
				m[0][3] = m[1][3] = m[2][3]           = zls::math::traits<T>::ZERO;
				m[0][0] = m[1][1] = m[2][2] = m[3][3] = zls::math::traits<T>::ONE;
			}

			void SetTranslate(T x, T y, T z)
			{
				identity.CopyTo(this);
				m[3][0] = x;
				m[3][1] = y;
				m[3][2] = z;
				m[3][3] = zls::math::traits<T>::ONE;
			}

			void SetViewLookatRH(const Vec3<T>& eyePos, const Vec3<T>& targetPos, const Vec3<T>& upDir)
			{
				Vec3<T> zDir = (eyePos - targetPos).normal();
				Vec3<T> xDir = (upDir.cross(zDir)).normal();
				Vec3<T> yDir = zDir.cross(xDir);

				identity.CopyTo(this);

				m[0][0] = xDir.x;
				m[0][1] = yDir.x;
				m[0][2] = zDir.x;

				m[1][0] = xDir.y;
				m[1][1] = yDir.y;
				m[1][2] = zDir.y;

				m[2][0] = xDir.z;
				m[2][1] = yDir.z;
				m[2][2] = zDir.z;

				m[3][0] = -(xDir*eyePos);
				m[3][1] = -(yDir*eyePos);
				m[3][2] = -(zDir*eyePos);
			}

			void SetViewLookatLH(const Vec3<T>& eyePos, const Vec3<T>& targetPos, const Vec3<T>& upDir)
			{
				Vec3<T> zDir = (targetPos - eyePos).normal();
				Vec3<T> xDir = (upDir.cross(zDir)).normal();
				Vec3<T> yDir = zDir.cross(xDir);

				identity.CopyTo(this);

				m[0][0] = xDir.x;
				m[0][1] = yDir.x;
				m[0][2] = xDir.x;

				m[1][0] = xDir.y;
				m[1][1] = yDir.y;
				m[1][2] = zDir.y;

				m[2][0] = xDir.z;
				m[2][1] = yDir.z;
				m[2][2] = zDir.z;

				m[3][0] = -(xDir*eyePos);
				m[3][1] = -(yDir*eyePos);
				m[3][2] = -(zDir*eyePos);
			}

			void _SymetricRH_DX(const T _near, const T _far)
			{
				const T nmf = _near - _far;
				m[2][2] = _far / nmf;
				m[3][2] = (_far*_near) / nmf;
			}

			void _SymetricRH_GL(const T _near, const T _far)
			{
				const T nmf = _near - _far;
				m[2][2] = (_far + _near) / nmf;
				m[3][2] = (2 * _far*_near) / nmf;
			}

			void _SymetricLH_DX(const T _near, const T _far)
			{
				const T nmf = _near - _far;
				m[2][2] = -_far / nmf;
				m[3][2] = (_far*_near) / nmf;
			}

			void _SymetricLH_GL(const T _near, const T _far)
			{
				const T fmn = _far - _near;
				m[2][2] = (_far + _near) / fmn;
				m[3][2] = (-2 * _far*_near) / fmn;
			}

			template<unsigned int renderer>
			void SetFrustumRH(const T _near, const T _far, const T _top, const T _bottom, const T _left, const T _right)
			{
				const T n2 = 2 * _near;
				const T rml = _right - _left;
				const T tmb = _top - _bottom;

				identity.CopyTo(this);

				m[0][0] = n2 / rml;
				m[1][1] = n2 / tmb;

				m[2][0] = (_right+_left) / rml;
				m[2][1] = (_top+_bottom) / tmb;

				switch (renderer)
				{
				case 0: _SymetricRH_DX(_near, _far); break;//D3D
				case 1: _SymetricRH_GL(_near, _far); break;//OGL
				}

				m[2][3] = -zls::math::traits<T>::ONE;
				m[3][3] = zls::math::traits<T>::ZERO;
			}

			template<unsigned int renderer>
			void SetSymetricFrustumRH(const T _near, const T _far, const T _top, const T _right)
			{
				identity.CopyTo(this);
				m[0][0] = _near / _right;
				m[1][1] = _near / _top;

				switch (renderer)
				{
				case 0: _SymetricRH_DX(_near, _far); break;//D3D
				case 1: _SymetricRH_GL(_near, _far); break;//OGL
				}

				m[2][3] = -zls::math::traits<T>::ONE;
				m[3][3] = zls::math::traits<T>::ZERO;
			}

			template<unsigned int renderer>
			void SetSymetricPerspectiveRH(const T _near, const T _far, const T _fovY, const T _aspect)
			{
				identity.CopyTo(this);
				const T halffovY = fovY / 2 * M_PI / 180.0;

				m[0][0] = zls::math::traits<T>::ONE / tan(halffovY);

				m[1][1] = m[0][0]*_aspect;

				switch (renderer)
				{
				case 0: _SymetricRH_DX(_near, _far);  break;//D3D
				case 1: _SymetricRH_GL(_near, _far); break;//OGL
				}

				m[2][3] = -zls::math::traits<T>::ONE;
				m[3][3] = zls::math::traits<T>::ZERO;
			}

			template<unsigned int renderer>
			void SetFrustumLH(const T _near, const T _far, const T _top, const T _bottom, const T _left, const T _right)
			{
				const T n2 = 2 * _near;
				const T rml = _right - _left;
				const T tmb = _top - _bottom;

				identity.CopyTo(this);

				m[0][0] = n2 / rml;
				m[1][1] = n2 / tmb;

				m[2][0] = (_right + _left) / rml;
				m[2][1] = (_top + _bottom) / tmb;

				switch (renderer)
				{
				case 0: _SymetricLH_DX(_near, _far); break;//D3D
				case 1: _SymetricLH_GL(_near, _far); break;//OGL
				}

				m[2][3] = -zls::math::traits<T>::ONE;
				m[3][3] = zls::math::traits<T>::ZERO;
			}

			template<unsigned int renderer>
			void SetSymetricFrustumLH(const T _near, const T _far, const T _top, const T _right)
			{
				identity.CopyTo(this);
				m[0][0] = _near / _right;
				m[1][1] = _near / _top;

				switch (renderer)
				{
				case 0: _SymetricLH_DX(_near, _far); break;//D3D
				case 1: _SymetricLH_GL(_near, _far); break;//OGL
				}

				m[2][3] = zls::math::traits<T>::ONE;
				m[3][3] = zls::math::traits<T>::ZERO;
			}

			template<unsigned int renderer>
			void SetSymetricPerspectiveLH(const T _near, const T _far, const T _fovY, const T _aspect)
			{
				identity.CopyTo(this);
				const T halffovY = fovY / 2 * M_PI / 180.0;
				m[0][0] = zls::math::traits<T>::ONE / tan(halffovY);
				m[1][1] = m[0][0] * _aspect;

				switch (renderer)
				{
				case 0: _SymetricLH_DX(_near, _far); break;//D3D
				case 1: _SymetricLH_GL(_near, _far); break;//OGL
				}

				m[2][3] = zls::math::traits<T>::ONE;
				m[3][3] = zls::math::traits<T>::ZERO;
			}

			void SetRotateX_RH(T degree)
			{
				identity.CopyTo(this);
				const T rad = T(degree * M_PI / 180.0);

				m[1][1] = zls::math::cos(rad);
				m[1][2] = -zls::math::sin(rad);
				m[2][1] = -m[1][2];
				m[2][2] = m[1][1];
			}

			void SetRotateY_RH(T degree)
			{
				identity.CopyTo(this);
				const T rad = T(degree * M_PI / 180.0);

				m[0][0] = zls::math::cos(rad);
				m[2][2] = m[0][0];

				m[0][2] = zls::math::sin(rad);
				m[2][0] = -m[0][2];
			}

			void SetRotateZ_RH(T degree)
			{
				identity.CopyTo(this);
				const T rad = T(degree * M_PI / 180.0);

				m[0][0] = zls::math::cos(rad);
				m[1][1] = m[0][0];

				m[0][1] = -zls::math::sin(rad);
				m[1][0] = -m[0][1];
			}

			void SetRotateX_LH(T degree)
			{
				identity.CopyTo(this);
				const T rad = T(degree * M_PI / 180.0);

				m[1][1] = zls::math::cos(rad);
				m[2][2] = m[1][1];

				m[1][2] = zls::math::sin(rad);
				m[2][1] = -m[1][2];
			}

			void SetRotateY_LH(T degree)
			{
				identity.CopyTo(this);
				const T rad = T(degree * M_PI / 180.0);

				m[0][0] = zls::math::cos(rad);
				m[2][2] = m[0][0];

				m[0][2] = -zls::math::sin(rad);
				m[2][0] = -m[0][2];
			}

			void SetRotateZ_LH(T degree)
			{
				identity.CopyTo(this);
				const T rad = T(degree * M_PI / 180.0);

				m[0][0] = zls::math::cos(rad);
				m[1][1] = m[0][0];

				m[0][1] = zls::math::sin(rad);
				m[1][0] = -m[0][1];
			}

			T* operator ()()
			{
				return &m[0][0];
			}

			Mat4 operator*(Mat4& o)
			{
				return Mat4(
					m[0][0] * o.m[0][0] + m[1][0] * o.m[0][1] + m[2][0] * o.m[0][2] + m[3][0] * o.m[0][3],
					m[0][0] * o.m[1][0] + m[1][0] * o.m[1][1] + m[2][0] * o.m[1][2] + m[3][0] * o.m[1][3],
					m[0][0] * o.m[2][0] + m[1][0] * o.m[2][1] + m[2][0] * o.m[2][2] + m[3][0] * o.m[2][3],
					m[0][0] * o.m[3][0] + m[1][0] * o.m[3][1] + m[2][0] * o.m[3][2] + m[3][0] * o.m[3][3],

					m[0][1] * o.m[0][0] + m[1][1] * o.m[0][1] + m[2][1] * o.m[0][2] + m[3][1] * o.m[0][3],
					m[0][1] * o.m[1][0] + m[1][1] * o.m[1][1] + m[2][1] * o.m[1][2] + m[3][1] * o.m[1][3],
					m[0][1] * o.m[2][0] + m[1][1] * o.m[2][1] + m[2][1] * o.m[2][2] + m[3][1] * o.m[2][3],
					m[0][1] * o.m[3][0] + m[1][1] * o.m[3][1] + m[2][1] * o.m[3][2] + m[3][1] * o.m[3][3],

					m[0][2] * o.m[0][0] + m[1][2] * o.m[0][1] + m[2][2] * o.m[0][2] + m[3][2] * o.m[0][3],
					m[0][2] * o.m[1][0] + m[1][2] * o.m[1][1] + m[2][2] * o.m[1][2] + m[3][2] * o.m[1][3],
					m[0][2] * o.m[2][0] + m[1][2] * o.m[2][1] + m[2][2] * o.m[2][2] + m[3][2] * o.m[2][3],
					m[0][2] * o.m[3][0] + m[1][2] * o.m[3][1] + m[2][2] * o.m[3][2] + m[3][2] * o.m[3][3],

					m[0][3] * o.m[0][0] + m[1][3] * o.m[0][1] + m[2][3] * o.m[0][2] + m[3][3] * o.m[0][3],
					m[0][3] * o.m[1][0] + m[1][3] * o.m[1][1] + m[2][3] * o.m[1][2] + m[3][3] * o.m[1][3],
					m[0][3] * o.m[2][0] + m[1][3] * o.m[2][1] + m[2][3] * o.m[2][2] + m[3][3] * o.m[2][3],
					m[0][3] * o.m[3][0] + m[1][3] * o.m[3][1] + m[2][3] * o.m[3][2] + m[3][3] * o.m[3][3]
					);
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

		const Mat4<float> Mat4<float>::identity(1,0,0,0,  0,1,0,0,  0,0,1,0,  0,0,0,1);
	}
}

#if 0
mat2, mat3, mat4						2x2, 3x3, 4x4 float matrix
mat2x2, mat2x3, mat2x4					2 - column float matrix of 2, 3, or 4 rows
mat3x2, mat3x3, mat3x4					3 - column float matrix of 2, 3, or 4 rows
mat4x2, mat4x3, mat4x4					4 - column float matrix of 2, 3, or 4 rows
dmat2, dmat3, dmat4						2x2, 3x3, 4x4 double - precision float matrix 
dmat2x2, dmat2x3, dmat2x4				2 - col. double - precision float matrix of 2, 3, 4 rows
dmat3x2, dmat3x3, dmat3x4				3 - col. double - precision float matrix of 2, 3, 4 rows 
dmat4x2, dmat4x3, dmat4x4				4 - column double - precision float matrix of 2, 3, 4 rows
#endif
