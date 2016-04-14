#version 440

uniform BlockName {
	mat4x4 proj;
	mat4x4 view;
	mat4x4 model;
} Block;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 tc;
out vec2 uv;
out vec3 fPosition;

void main()
{
	float near = 1.0f;
	float far = 10.0f;
	float horiz = 1.0f;
	float vert = 1.0f;

	vec3 dlight = {-1.0, -1.0, -1.0};

	mat4x4 mv = (Block.view * Block.model);

	fPosition = (mv * vec4(pos, 1.0)).xyz;
	gl_Position = Block.proj * mv * vec4(pos, 1.0);

	uv = vec2(tc.x, 1.0-tc.y);
}
