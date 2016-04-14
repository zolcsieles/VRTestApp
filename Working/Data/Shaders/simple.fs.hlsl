//https://msdn.microsoft.com/en-us/library/windows/desktop/ff471376(v=vs.85).aspx
Texture2D tex;

SamplerState texState
{
	Filter = MIN_MAP_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct TResult
{
	float4 Position : SV_POSITION;
	float3 fPosition : POSITION;
	float2 uv : TEXCOORD;
};

float4 main(TResult inp) : SV_TARGET
{
	float3 lightDir = normalize(float3(1.0, 1.0, 1.0));
	float3 normalEye = normalize(cross(ddy_coarse(inp.fPosition), ddx_coarse(inp.fPosition)));
	return tex.Sample(texState, inp.uv) * saturate(dot(lightDir,normalEye));
}
