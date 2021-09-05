struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 Texcoord : TEXCOORD;
};

cbuffer VP : register(b1)
{
    float4x4 PV;
    float4x4 invPV;
    float3 cameraPos;
}

cbuffer Light : register(b0)
{
    float3 lightDir;
    float3 lightColor;
};

Texture2D albedoRougnessTexture : register(t0);
Texture2D normalMetallicTexture : register(t1);
Texture2D aoTexture : register(t2);
Texture2D depthTexture : register(t3);

sampler linearSample;
static const float PI = 3.14159265359;
//�t���l���̎��i�ߎ��j�@���ˋ��ܗ�
float3 fresnelSchlick(float cosTheta,float3 F0)
{
    return F0 + (1.0f - F0) * pow(clamp(1.0 - cosTheta, 0.0f, 1.0f), 5.0f);

}

float DistributionGGX(float3 N,float3 H,float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0f);
    float NdotH2 = NdotH * NdotH;
    
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = PI * denom * denom;
    
    return num / denom;
}

float GeometorySchlickGGX(float NdotV,float roughness)
{
    float r = (roughness + 1.0f);
    float k = (r * r) / 8.0f;
    
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return num / denom;
}

float GeometorySimith(float3 N,float3 V,float3 L ,float roughness)
{
    float NdotV = max(dot(N, V), 0.0f);
    float NdotL = max(dot(N, L), 0.0f);
    
    float ggx2 = GeometorySchlickGGX(NdotV, roughness);
    float ggx1 = GeometorySchlickGGX(NdotL, roughness);
    
    return ggx2 * ggx1;

}
float4 main(VSOutput input) : SV_TARGET
{
    uint2 intpos = input.Position.xy;
    float3 albedo = albedoRougnessTexture[intpos].xyz;
    float3 normal = normalMetallicTexture[intpos].xyz;
    float ao = aoTexture[intpos].x;
    float rougness = albedoRougnessTexture[intpos].w;
    float metallic = normalMetallicTexture[intpos].w / 2.0f + 0.5f;
    
    float z = depthTexture[intpos].x;
    float4 screenPos = float4(input.Position.xy, z, 1.0f);
    float4 worldPosition = mul(screenPos, invPV);
    worldPosition /= worldPosition.w;
    
    
    float3 N = normalize(normal);
    float3 V = normalize(cameraPos - worldPosition.xyz);
    
    float3 L = normalize(lightDir);
    float3 H = normalize(V + N);
    //cos���֌W�Ȃ��̂Ń��f�B�A���X�H
    float3 radiance = lightColor;
    //F0��0.04�����l�ŉ��肵�Ċȗ���
    float3 F0 = float3(0.04f.rrr);
    //F0��albedo�Ő��`��ԁ@��{�I�Ƀ��^���b�N��1or0�Ȃ̂łǂ��炩�̒l
    F0 = lerp(F0, albedo, metallic);
    
    
    
    //�N�b�N�g�����XBRDF(�X�y�L�������j
    float NDF = DistributionGGX(N, H, rougness);
    float G = GeometorySimith(N, V, L, rougness);
    float3 F = fresnelSchlick(max(dot(H, V), 0.0f), F0);
    
    //���ʔ��˂Ɗg�U���˂ŕ����i�G�l���M�[�ۑ��j
    float3 kS = F;
    float3 kD = float3(1.0.rrr) - kS;
    //���^���b�N1.0���Ɗg�U���˂Ȃ�
    kD *= 1.0 - metallic;
    
    //�X�y�L�������̌v�Z
    float3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    float3 specular = numerator / denominator;
    float NdotL = max(dot(N, L), 0.0);
    //���K��lambert
    //�����_�����O������
    float3 Lout = (kD * albedo / PI + specular) * radiance * NdotL;
    
    //����
    float3 ambient = float3(0.03.rrr) * albedo * ao;
    float3 color = ambient + Lout;
    
    //HDR�␳
    color = color / (color + float3(1.0.rrr));
    color = pow(color, float3(1.0.rrr / 2.2));
    
    
	return float4(color, 1.0f);
}