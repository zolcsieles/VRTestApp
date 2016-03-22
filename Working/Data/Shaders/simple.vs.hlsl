struct TResult
{
	float4 Position : SV_POSITION;
	float3 Color : COLOR;
};

TResult main(float4 pos : POSITION, float3 color : COLOR, float3 color2 : COLOR1)
{
	TResult ret;

	ret.Position = pos;
	ret.Color = color + color2;

	return ret;
}
