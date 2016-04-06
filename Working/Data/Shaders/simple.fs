#version 440

layout(location = 0) out vec4 color;
uniform sampler2D tex;
in vec3 col;
in vec2 uv;

void main()
{
	color = vec4(col.rgb, 0.5) + texture(tex, uv);
}
