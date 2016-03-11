#version 440

in vec2 uv;
out vec3 color;

uniform sampler2D texLeft;
uniform sampler2D texRight;
uniform float leftFirst; 

void main()
{
	float y = uv.y * 600 + leftFirst;
	y /= 2;
	float f = mod(y, 1);
	color += texture(texLeft, uv).rgb * (1.0 - f) + texture(texRight, uv).rgb * (f);
}
