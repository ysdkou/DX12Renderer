struct VSInput {
	float4 Position: POSITION;
};
struct VSOutput {
	float4 Position:SV_POSITION;
};

cbuffer MVP:register(b1)
{
	float4x4 world;
	float4x4 view;
	float4x4 projection;
}
VSOutput main(VSInput input)
{
	VSOutput result = (VSOutput)0;
	float4x4 mvpMatrix = mul(world, mul(view, projection));
	result.Position = mul(input.Position, mvpMatrix);
	return result;
}