#version 440

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 color2;
out vec3 col;

void main()
{
	gl_Position = vec4(pos, 1.0);
	col = color + color2;
}
