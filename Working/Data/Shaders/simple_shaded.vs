#version 440

uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projMat;

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 v_uv;
out vec3 fPosition;
out vec2 uv;

void main()
{
	fPosition = (viewMat * modelMat * vec4(position, 1.0)).xyz;
	gl_Position = projMat * viewMat * modelMat * vec4(position, 1.0);
	uv = v_uv;
}
