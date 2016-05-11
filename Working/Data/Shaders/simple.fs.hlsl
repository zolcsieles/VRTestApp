//https://msdn.microsoft.com/en-us/library/windows/desktop/ff471376(v=vs.85).aspx
Texture2D tex;

SamplerState texState;

struct TResult
{
	float4 Position : SV_POSITION;
	float3 _position : POSITION;
	float2 _uv : TEXCOORD;
};

float4 main(TResult inp) : SV_TARGET
{
	float3 lightDir = normalize(float3(1.0, 1.0, 1.0));
	float3 normalEye = normalize(cross(ddy_coarse(inp._position), ddx_coarse(inp._position)));
	return float4((tex.Sample(texState, inp._uv) * saturate(dot(lightDir,normalEye))).xyz, 0.0f);
}
