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
    float4 shadowPos : TEXCOORD3;
};

cbuffer Transform : register(b0)
{
    matrix world;
    matrix view;
    matrix proj;
};

cbuffer LightData : register(b1)
{
    float3 lightDir;
    float ambientStrength;

    float3 cameraPos;
    float pad0;

    float3 lightColor;
    float pad1;
};

cbuffer MaterialData : register(b2)
{
    float3 specularColor;
    float shininess;

    float specStrength;
    float3 padding;
};

cbuffer ShadowData : register(b3)
{
    matrix lightViewProj;
};

Texture2D diffuseTexture : register(t0);
Texture2D shadowMap : register(t1);

SamplerState samLinear : register(s0);
SamplerState shadowSampler : register(s1);

struct VS_OUT_SHADOW
{
    float4 pos : SV_POSITION;
};

VS_OUT_SHADOW VSShadow(VS_IN input)
{
    VS_OUT_SHADOW output;

    float4 worldPos = mul(float4(input.pos, 1.0f), world);
    output.pos = mul(worldPos, lightViewProj);

    return output;
}

float4 PSShadow() : SV_Target
{
    return float4(1, 1, 1, 1);
}

PS_IN VSMain(VS_IN input)
{
    PS_IN output;

    float4 worldPos = mul(float4(input.pos, 1.0f), world);
    float4 viewPos = mul(worldPos, view);
    float4 clipPos = mul(viewPos, proj);

    output.pos = clipPos;
    output.col = input.col;
    output.tex = input.tex;
    output.worldPos = worldPos.xyz;
    output.worldNormal = normalize(mul(input.normal, (float3x3) world));
    output.shadowPos = mul(worldPos, lightViewProj);

    return output;
}

float CalculateShadow(float4 shadowPos)
{
    float3 projCoords = shadowPos.xyz / shadowPos.w;

    float2 uv;
    uv.x = projCoords.x * 0.5f + 0.5f;
    uv.y = -projCoords.y * 0.5f + 0.5f;

    float currentDepth = projCoords.z;

    if (uv.x < 0.0f || uv.x > 1.0f || uv.y < 0.0f || uv.y > 1.0f)
        return 1.0f;

    float closestDepth = shadowMap.Sample(shadowSampler, uv).r;

    float bias = 0.001f;

    return (currentDepth - bias <= closestDepth) ? 1.0f : 0.3f;
}


//float4 PSMain(PS_IN input) : SV_Target
//{
//    float shadow = CalculateShadow(input.shadowPos);
//    return float4(shadow, shadow, shadow, 1.0f);
//}

float4 PSMain(PS_IN input) : SV_Target
{
    float3 N = normalize(input.worldNormal);
    float3 L = normalize(-lightDir);
    float3 V = normalize(cameraPos - input.worldPos);
    float3 R = reflect(-L, N);

    float diff = max(dot(N, L), 0.0f);
    float spec = pow(max(dot(V, R), 0.0f), shininess);

    float shadow = CalculateShadow(input.shadowPos);

    float3 ambient = ambientStrength * lightColor;
    float3 diffuse = diff * lightColor * shadow;
    float3 specular = specStrength * spec * specularColor * lightColor * shadow;

    float4 texColor = diffuseTexture.Sample(samLinear, input.tex);

    float3 finalLight = ambient + diffuse + specular;
    float3 finalColor = texColor.rgb * input.col.rgb * finalLight;

    return float4(finalColor, texColor.a * input.col.a);
}