#version 440

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec2 v_uv;

out vec2 UV;

void main()
{
	gl_Position = vec4(v_position, 1.0);
	UV = v_uv;
}
