#version 440

out vec4 retcolor;
uniform sampler2D tex;

in vec3 _position;
in vec2 _uv;

void main()
{
	vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
	vec3 normalEye = normalize(cross(dFdx(_position), dFdy(_position)));
	retcolor = vec4((texture(tex, _uv) * clamp(dot(lightDir, normalEye), 0.0f, 1.0f)).xyz, 0.0f);
}
