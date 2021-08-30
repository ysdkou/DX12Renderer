struct VSOutput
{
    float4 Position : SV_POSITION;
    float4 Normal : NORMAL;
};

cbuffer Matrial : register(b2)
{
    float3 albedo;
    float metallic;
    float roughness;
    float ao;
}

struct PSOutput 
{
    float4 AlbedoRoughness : SV_TARGET0;
    float4 NormalMetallic : SV_TARGET1;
    float4 AO : SV_TARGET2;
};


PSOutput main(VSOutput input) 
{
    PSOutput output;
    //UNORM
    output.AlbedoRoughness = float4(albedo, roughness);
    //SNORM
    output.NormalMetallic = float4(0, 1.0, 0, 1);
    //UNORM
    output.AO = float4(0,0,1.0,1.0);
    return output;
}