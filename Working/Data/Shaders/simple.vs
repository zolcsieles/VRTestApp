#version 440

layout(location = 0) in vec3 v_pos;
layout(location = 1) in vec3 v_col;
layout(location = 2) in vec3 v_col2;
out vec3 col;

void main()
{
	gl_Position = vec4(v_pos, 1.0);
	col = v_col + v_col2;
}
