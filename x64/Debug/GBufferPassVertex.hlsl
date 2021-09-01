struct VSInput
{
    float4 Position : POSITION;
    float4 Normal : NORMAL;
};
struct VSOutput
{
    float4 Position : SV_POSITION;
    float4 Normal : NORMAL;
};
cbuffer VP : register(b1)
{
    float4x4 PV;
    float4x4 invPV;
    float3 cameraPos;
}


cbuffer Model : register(b3)
{
    float4x4 model;
    float4x4 forNormal;
}
VSOutput main(VSInput input)
{
    VSOutput result = (VSOutput) 0;
    float4x4 mvpMatrix = mul(model, PV);
    result.Position = mul(input.Position, mvpMatrix);
    result.Normal = mul(input.Normal,forNormal);
    return result;
}