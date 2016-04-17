#version 440

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv;
//out vec4 pos;
out vec2 o_uv;

void main()
{
	gl_Position = vec4(pos, 0.0f, 1.0f);
	o_uv = vec2(uv.x, 1-uv.y);
}
