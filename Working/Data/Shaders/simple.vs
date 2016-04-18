#version 440

uniform BlockName {
	mat4x4 proj;
	mat4x4 view;
	mat4x4 model;
} Block;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;

out vec2 _uv;
out vec3 _position;

void main()
{
	_uv = vec2(uv.x, 1.0 - uv.y);

	mat4x4 mv = Block.view * Block.model;
	vec4 temp_position = mv * vec4(pos, 1.0);

	_position = temp_position.xyz;
	gl_Position = Block.proj * temp_position;
}
