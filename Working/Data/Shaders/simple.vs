#version 440

uniform BlockName {
	mat4x4 proj;
	mat4x4 view;
	mat4x4 model;
} Block;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 tc;
out vec3 col;
out vec2 uv;

void main()
{
	float near = 1.0f;
	float far = 10.0f;
	float horiz = 1.0f;
	float vert = 1.0f;

	mat4x4 mvp = Block.proj * (Block.view * Block.model);
	gl_Position = mvp * vec4(pos, 1.0);

	col = color;
	uv = vec2(tc.x, 1.0-tc.y);
}
