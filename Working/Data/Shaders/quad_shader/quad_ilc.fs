#version 440

in vec2 UV;
out vec3 color;

uniform sampler2D texLeft;
uniform sampler2D texRight;
uniform float swapEyes; 

void main()
{
	//Interlaced
	vec2 uv = UV;
	float y = fma(UV.y, 600, swapEyes) / 2.0f;

	//Common
	float ew = round(fract(y));
	color = mix(texture(texLeft, uv).rgb, texture(texRight, uv).rgb, ew);
}
