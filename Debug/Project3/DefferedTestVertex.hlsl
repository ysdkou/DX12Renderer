struct VSInput {
	float4 Position: POSITION;
};
struct VSOutput {
	float4 Position:SV_POSITION;
};
cbuffer VP:register(b1)
{
	float4x4 view;
	float4x4 projection;
    float3 cameraPos;
}
cbuffer Model : register(b3)
{
    float4x4 model;
}
VSOutput main(VSInput input)
{
	VSOutput result = (VSOutput)0;
	float4x4 mvpMatrix = mul(model, mul(view, projection));
	result.Position = mul(input.Position, mvpMatrix);
	return result;
}