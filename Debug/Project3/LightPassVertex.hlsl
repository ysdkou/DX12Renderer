struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 Texcoord : TEXCOORD;
};

struct VSInput
{
    float4 Position : POSITION;
    float2 Texcoord : TEXCOORD;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.Position = input.Position;
    output.Texcoord = input.Texcoord;
    return output;
}