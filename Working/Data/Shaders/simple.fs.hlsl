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
	float3 Color : COLOR;
	float2 uv : TEXCOORD;
};

float4 main(TResult inp) : SV_TARGET
{
	return float4(inp.Color.xyz, 0.5f) + tex.Sample(texState, inp.uv);
}
