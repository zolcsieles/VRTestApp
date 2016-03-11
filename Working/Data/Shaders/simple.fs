#version 440

layout(location = 0) out vec3 color;

uniform vec3 faceColor;

void main()
{
	color = faceColor;
}
