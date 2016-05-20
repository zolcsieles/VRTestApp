#pragma once

struct _declspec(align(8)) VS_Constant
{
	zls::math::mat4 proj;
	zls::math::mat4 view;
	zls::math::mat4 model;
};
