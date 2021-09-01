struct VSOutput {
	float4 Position:SV_POSITION;
};
cbuffer VP : register(b1)
{
    float3 cameraPos;
    float4x4 view;
    float4x4 projection;
};

cbuffer Light: register(b0) {
	float3 dir;
	float3 color;
};

cbuffer Matrial:register(b2)
{
	float3 albedo;
	float metallic;
	float roughness;
	float ao;
}

float4 main(VSOutput input) :SV_TARGET
{
	return float4(albedo,1.0);
}