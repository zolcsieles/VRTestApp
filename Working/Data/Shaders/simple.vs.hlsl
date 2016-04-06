cbuffer InputBuffer
{
	float4x4 proj;
	float3 shift;
};

struct Input
{
	float3 pos : POSITION;
	float3 color : COLOR;
	float3 color2 : COLOR1;
	float2 tc : TEXCOORD;
};

struct TResult
{
	float4 Position : SV_POSITION;
	float3 Color : COLOR;
	float2 uv : TEXCOORD;
};

TResult main(Input inp)
{
	TResult ret;

	float near = 1.0f;
	float far = 10.0f;
	float horiz = 1.0f;
	float vert = 1.0f;

	
	float4x4 mtx = float4x4(
		near / horiz, 0.0f, 0.0f, 0.0f,
		0.0f, near / vert, 0.0f, 0.0f,
		0.0f, 0.0f, -(far + near) / (far - near), -(2 * far*near) / (far - near),
		0.0f, 0.0f, -1.0f, 0.0f
		);
		
	float3 shift2 = float3(0.0f, 0.0f,-1.0);

	ret.Position = mul(float4(inp.pos + shift2 + shift, 1.0f), proj);
	ret.Color = inp.color * inp.tc.x + inp.color2 * inp.tc.y;
	ret.uv = inp.tc;

	return ret;
}
