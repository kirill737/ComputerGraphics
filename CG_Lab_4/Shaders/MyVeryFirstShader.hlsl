struct VS_IN
{
    float3 pos : POSITION0;
    float4 col : COLOR0;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    float2 tex : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
    float3 worldNormal : TEXCOORD2;
};

cbuffer Transform : register(b0)
{
    matrix world;
    matrix view;
    matrix proj;
};

cbuffer LightData : register(b1)
{
    float3 lightPos;
    float ambientStrength;

    float3 cameraPos;
    float specPower;

    float3 lightColor;
    float specStrength;
};

Texture2D diffuseTexture : register(t0);
SamplerState samLinear : register(s0);

PS_IN VSMain(VS_IN input)
{
    PS_IN output;

    float4 worldPos = mul(float4(input.pos, 1.0f), world);
    float4 viewPos = mul(worldPos, view);
    float4 clipPos = mul(viewPos, proj);

    output.pos = clipPos; // обязательно float4
    output.col = input.col;
    output.tex = input.tex;
    output.worldPos = worldPos.xyz;
    output.worldNormal = normalize(mul(float4(input.normal, 0.0f), world).xyz);

    return output;
}

float4 PSMain(PS_IN input) : SV_Target
{
    float3 N = normalize(input.worldNormal);
    float3 L = normalize(lightPos - input.worldPos);
    float3 V = normalize(cameraPos - input.worldPos);
    float3 R = reflect(-L, N);

    float diff = max(dot(N, L), 0.0f);
    float spec = pow(max(dot(V, R), 0.0f), specPower);

    float3 ambient = ambientStrength * lightColor;
    float3 diffuse = diff * lightColor;
    float3 specular = specStrength * spec * lightColor;

    float4 texColor = diffuseTexture.Sample(samLinear, input.tex);

    float3 finalLight = ambient + diffuse + specular;
    float3 finalColor = texColor.rgb * input.col.rgb * finalLight;

    return float4(finalColor, texColor.a * input.col.a);
}