#version 440

out vec4 retcolor;
uniform sampler2D tex;

in vec3 fPosition;
in vec2 uv;
in vec3 tcolor;

void main()
{
	vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
	vec3 normalEye = normalize(cross(dFdx(fPosition), dFdy(fPosition)));
	retcolor = texture(tex, uv) * clamp(dot(lightDir, normalEye), 0.0, 1);
}
