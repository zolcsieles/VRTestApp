float4 main(float4 position : SV_POSITION, float3 color : COLOR) : SV_TARGET
{
	return float4(color.xyz, 0.5f);
}
