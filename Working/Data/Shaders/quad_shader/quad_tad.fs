#version 440

in vec2 UV;
out vec3 color;

uniform sampler2D texLeft;
uniform sampler2D texRight;
uniform float swapEyes; 

void main()
{
	//Top And Down - Over and Under
	vec2 uv = vec2(UV.x, UV.y*2);
	float y = (swapEyes+1) / 2 + UV.y;

	//Common
	float ew = round(fract(y));
	color = mix(texture(texLeft, uv).rgb, texture(texRight, uv).rgb, ew);
}
