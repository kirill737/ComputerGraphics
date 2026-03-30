struct VS_IN
{
    float4 pos : POSITION0;
    float4 col : COLOR0;
    float2 tex : TEXCOORD0;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
    float2 tex : TEXCOORD0;
};

cbuffer Transform : register(b0)
{
    matrix world;
    matrix view;
    matrix proj;
};

Texture2D diffuseTexture : register(t0);
SamplerState samLinear : register(s0);

PS_IN VSMain(VS_IN input)
{
    PS_IN output;

    float4 pos = mul(input.pos, world);
    pos = mul(pos, view);
    pos = mul(pos, proj);

    output.pos = pos;
    output.col = input.col;
    output.tex = input.tex;

    return output;
}

//float4 PSMain(PS_IN input) : SV_Target
//{
//    float4 col = input.col;
//    return col;
//}
float4 PSMain(PS_IN input) : SV_Target
{
    float4 texColor = diffuseTexture.Sample(samLinear, input.tex);
    return texColor * input.col;
}