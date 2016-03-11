#version 440

in vec2 UV;
out vec3 color;

uniform sampler2D texLeft;
uniform sampler2D texRight;
uniform float swapEyes; 

void main()
{
	//Left And Right - Side by Side
	vec2 uv = vec2(UV.x*2, UV.y);
	float y = swapEyes / 2 + UV.x;

	//Common
	float ew = round(fract(y));
	color = mix(texture(texLeft, uv).rgb, texture(texRight, uv).rgb, ew);
}
