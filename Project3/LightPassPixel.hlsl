struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 Texcoord : TEXCOORD;
};

Texture2D albedoRougnessTexture : register(t0);
Texture2D normalMetallicTexture : register(t1);
Texture2D aoTexture : register(t2);
Texture2D gDepth : register(t3);

sampler linearSample;

float4 main(VSOutput input) : SV_TARGET
{
    uint2 intpos = input.Position.xy;
    float3 albedo = albedoRougnessTexture[intpos].xyz;
    float3 normal = normalMetallicTexture[intpos].xyz;
    float3 ao = aoTexture[intpos].xyz;
    
    float3 sum = albedo + normal + ao;
	return float4(sum, 1.0f);
}