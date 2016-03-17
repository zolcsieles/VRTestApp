#version 440

layout(location = 0) out vec3 color;

uniform sampler2D tex;
//uniform vec3 faceColor;
in vec3 fPosition;
in vec2 uv;

void main()
{
	vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
	vec3 normalEye = normalize(cross(dFdx(fPosition), dFdy(fPosition)));
	color = texture(tex, uv).rgb * clamp(dot(lightDir, normalEye), 0.25, 1);
}
