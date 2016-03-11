#version 440

uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projMat;

layout(location = 0) in vec3 position;

void main()
{
	gl_Position = projMat * viewMat * modelMat * vec4(position, 1.0);
}
