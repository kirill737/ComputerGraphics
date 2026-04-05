cbuffer Transform : register(b0)
{
    matrix world;
    matrix view;
    matrix proj;
};

cbuffer ShadowData : register(b3)
{
    matrix lightViewProj;
};

struct VS_IN
{
    float3 pos : POSITION0;
    float4 col : COLOR0;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
};

VS_OUT VSShadow(VS_IN input)
{
    VS_OUT output;

    float4 worldPos = mul(float4(input.pos, 1.0f), world);
    output.pos = mul(worldPos, lightViewProj);

    return output;
}