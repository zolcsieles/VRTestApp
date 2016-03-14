#version 440

layout(location = 0) out vec3 color;

uniform vec3 faceColor;
in vec3 fPosition;
//in vec3 aPosition;

void main()
{
	vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
	vec3 normalEye = normalize(cross(dFdx(fPosition), dFdy(fPosition)));
	
	//vec3 c = aPosition;
	color = faceColor * clamp(dot(lightDir, normalEye), 0.25, 1);
}
