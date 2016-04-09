#version 440

uniform BlockName {
	mat4x4 proj;
	vec3 shift;
} Block;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 color2;
layout(location = 3) in vec2 tc;
out vec3 col;
out vec2 uv;

void main()
{
	float near = 1.0f;
	float far = 10.0f;
	float horiz = 1.0f;
	float vert = 1.0f;

	mat4x4 mtx =
	{
		{ near / horiz, 0.0f, 0.0f, 0.0f },
		{ 0.0f, near / vert, 0.0f, 0.0f },
		{ 0.0f, 0.0f, -(far + near) / (far - near), -(2 * far*near) / (far - near) },
		{ 0.0f, 0.0f, -1.0f, 0.0f }
	};

	vec3 shift2 = { 0.0f, 0.0f,-1.0f };

	gl_Position = vec4(pos + shift2 + Block.shift, 1.0) * Block.proj;
	col = color * tc.x + color2 * tc.y;
	uv = vec2(tc.x, 1.0-tc.y);
}
