#version 440

uniform sampler2D tex;
in vec2 o_uv;
out vec4 color;

void main()
{
	color = texture(tex, o_uv);
}
