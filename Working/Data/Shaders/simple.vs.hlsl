cbuffer InputBuffer
{
	float4x4 proj;
	float4x4 view;
	float4x4 model;
};

struct Input
{
	float3 pos : POSITION;
	float2 uv : TEXCOORD;
};

struct TResult
{
	float4 Position : SV_POSITION;
	float3 _position : POSITION;
	float2 _uv : TEXCOORD;
};

TResult main(Input inp)
{
	TResult ret;
	ret._uv = inp.uv;

	float4x4 mv = mul(view, model);
	float4 temp_position = mul(mv, float4(inp.pos, 1.0f));

	ret._position = mul(mv, float4(inp.pos, 1.0f)).xyz;
	ret.Position = mul(proj, temp_position);

	return ret;
}
