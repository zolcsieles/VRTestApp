#pragma once

#include "zls_math/zls_math.h"

class Actor
{
private:
	typedef float vptype;
	typedef zls::math::Vec2<vptype> vec2;
	typedef zls::math::Vec3<vptype> vec3;

	zls::math::Vec3<vptype> mPosition;
	zls::math::Vec2<vptype> mViewAngles;

public:
	Actor(vptype _height) : mPosition(0, 0, _height), mViewAngles(0, 0)
	{}

	Actor(vec3& _position, vec2& _viewangles) : mPosition(_position), mViewAngles(_viewangles)
	{}

	void SetViewAngles(vec2& _viewangles)
	{
		mViewAngles = _viewangles;
	}

	void AddViewAngles(vec2& _viewangles)
	{
		mViewAngles += _viewangles;
	}

	void SetPosition(vec3& _position)
	{
		mPosition = _position;
	}

	void AddPosition(vec3& _position)
	{
		mPosition += _position;
	}

	vptype GetPositionX() { return mPosition.x; }
	vptype GetPositionY() { return mPosition.y; }
	vptype GetPositionZ() { return mPosition.z; }

};
