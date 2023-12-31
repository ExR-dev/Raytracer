struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 texCoord : TEXCOORD0;
	float3 color : COLOR0;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	return float4(input.color.xy, 0.0f, 1.0f);
}
