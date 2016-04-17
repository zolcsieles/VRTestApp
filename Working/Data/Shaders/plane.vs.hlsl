struct Input
{
	float2 pos : POSITION;
	float2 uv : TEXCOORD;
};

struct TResult
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

TResult main(Input inp)
{
	TResult ret;
	ret.pos = float4(inp.pos.xy, 0.0f, 1.0f);
	ret.uv = inp.uv;
	return ret;
}
