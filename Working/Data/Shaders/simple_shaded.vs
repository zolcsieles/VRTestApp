#version 440

uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projMat;

layout(location = 0) in vec3 position;
out vec3 fPosition;
//out vec3 aPosition;

void main()
{
	fPosition = (viewMat * modelMat * vec4(position, 1.0)).xyz;
	//aPosition = (position+vec3(5.0, 5.0, 5.0))/10.0;
	gl_Position = projMat * viewMat * modelMat * vec4(position, 1.0);
}
