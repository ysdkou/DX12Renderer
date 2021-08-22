struct VSOutput {
	float4 Position:SV_POSITION;
};

cbuffer Light: register(b0) {
	float3 dir;
	float3 color;
};

cbuffer Matria:register(b2)
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