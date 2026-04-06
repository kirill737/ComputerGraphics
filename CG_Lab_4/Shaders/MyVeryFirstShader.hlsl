#define MAX_POINT_LIGHTS 8

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

struct VS_OUT_SHADOW
{
    float4 pos : SV_POSITION;
};

struct PointLightData
{
    float3 pos;
    float range;

    float3 color;
    float intensity;
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

    PointLightData pointLights[MAX_POINT_LIGHTS];
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
Texture2D shadowPatternTexture : register(t2);

SamplerState samLinear : register(s0);
SamplerState shadowSampler : register(s1);

VS_OUT_SHADOW VSShadow(VS_IN input)
{
    VS_OUT_SHADOW output;

    float4 worldPos = mul(float4(input.pos, 1.0f), world);
    output.pos = mul(worldPos, lightViewProj);

    return output;
}

float4 PSShadow() : SV_Target
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
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

float CalculateShadowRaw(float4 shadowPos)
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

    return (currentDepth - bias <= closestDepth) ? 1.0f : 0.0f;
}

float CalculatePatternedShadow(PS_IN input)
{
    float shadowRaw = CalculateShadowRaw(input.shadowPos);

    // Если пиксель не в тени — полный свет
    if (shadowRaw > 0.5f)
        return 1.0f;

    // Узор привязан к миру/полу через worldPos.xz
    float2 patternUV = input.worldPos.xz * 0.5f;

    float pattern = shadowPatternTexture.Sample(samLinear, patternUV).r;

    float patternedShadow = lerp(0.20f, 0.75f, pattern);

    return patternedShadow;
}

float4 PSMain(PS_IN input) : SV_Target
{
    float3 N = normalize(input.worldNormal);
    float3 V = normalize(cameraPos - input.worldPos);

    float4 texColor = diffuseTexture.Sample(samLinear, input.tex);

    //Directional light 
    float3 L = normalize(-lightDir);
    float3 R = reflect(-L, N);

    float diff = max(dot(N, L), 0.0f);
    float spec = pow(max(dot(V, R), 0.0f), shininess);

    float shadowFactor = CalculatePatternedShadow(input);

    float3 ambient = ambientStrength * lightColor;
    float3 diffuse = diff * lightColor * shadowFactor;
    float3 specular = specStrength * spec * specularColor * lightColor * shadowFactor;

    // Point lights
    float3 pointDiffuseSum = 0.0f;
    float3 pointSpecularSum = 0.0f;

    [unroll]
    for (int i = 0; i < MAX_POINT_LIGHTS; i++)
    {
        float3 toLight = pointLights[i].pos - input.worldPos;
        float dist = length(toLight);

        if (pointLights[i].intensity > 0.0f && dist < pointLights[i].range)
        {
            float3 LP = normalize(toLight);
            float atten = 1.0f - saturate(dist / pointLights[i].range);

            float pointDiff = max(dot(N, LP), 0.0f);

            float3 pointR = reflect(-LP, N);
            float pointSpec = pow(max(dot(V, pointR), 0.0f), shininess);

            pointDiffuseSum += pointDiff * pointLights[i].color * atten * pointLights[i].intensity;
            pointSpecularSum += specStrength * pointSpec * specularColor * pointLights[i].color * atten * pointLights[i].intensity;
        }
    }

    float3 finalLight = ambient + diffuse + specular + pointDiffuseSum + pointSpecularSum;
    float3 finalColor = texColor.rgb * input.col.rgb * finalLight;

    return float4(finalColor, texColor.a * input.col.a);
}