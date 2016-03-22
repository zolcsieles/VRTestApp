#version 440

layout(location = 0) out vec4 color;
in vec3 col;

void main()
{
	//color = vec4(0.0, 0.5, 1.3, 0.5);
	color = vec4(col.rgb, 0.5);
}
