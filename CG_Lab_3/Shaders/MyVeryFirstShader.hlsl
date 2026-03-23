struct VS_IN
{
    float4 pos : POSITION0;
    float4 col : COLOR0;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
};

cbuffer Transform : register(b0)
{
    matrix world;
    matrix view;
    matrix proj;
};

PS_IN VSMain(VS_IN input)
{
    PS_IN output;

    float4 pos = mul(input.pos, world);
    pos = mul(pos, view);
    pos = mul(pos, proj);

    output.pos = pos;
    output.col = input.col;

    return output;
}

float4 PSMain(PS_IN input) : SV_Target
{
    float4 col = input.col;
#ifdef TEST
    //if (input.pos.x > 400) col = TCOLOR;
#endif
    return col;
}