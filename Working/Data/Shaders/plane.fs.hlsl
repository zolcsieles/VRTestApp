Texture2D tex;
SamplerState texState;

struct TResult
{
	float4 pos : POSITION;
	float2 uv : TEXCOORD;
};

float4 main(TResult inp) : SV_TARGET
{
	return tex.Sample(texState, inp.uv);
}
