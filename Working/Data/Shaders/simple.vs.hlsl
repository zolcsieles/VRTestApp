cbuffer InputBuffer
{
	float4x4 proj;
	float4x4 view;
	float4x4 model;
};

struct Input
{
	float3 pos : POSITION;
	float3 color : COLOR;
	float2 tc : TEXCOORD;
};

struct TResult
{
	float4 Position : SV_POSITION;
	float2 uv : TEXCOORD;
};

TResult main(Input inp)
{
	TResult ret;

	float4x4 mvp = mul(mul(proj, view), model);

	float near = 1.0f;
	float far = 10.0f;
	float horiz = 1.0f;
	float vert = 1.0f;

	ret.Position = mul(mvp, float4(inp.pos, 1.0f));
	ret.uv = inp.tc;

	return ret;
}
