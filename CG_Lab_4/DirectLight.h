#pragma once

#include "GameComponent.h"
#include <windows.h>
#include <d3d11.h>

using namespace DirectX::SimpleMath;


static constexpr int MAX_PROJECTILE_LIGHTS = 16;

struct PointLightData
{
	DirectX::SimpleMath::Vector3 pos;
	float range;

	DirectX::SimpleMath::Vector3 color;
	float intensity;
};

struct LightBufferData
{
	Vector3 lightDir;
	float ambientStrength;

	Vector3 cameraPos;
	float pad0 = 0.0f;

	Vector3 lightColor;
	float pad1 = 0.0f;

	PointLightData pointLights[MAX_PROJECTILE_LIGHTS];
};
struct ProjectileLight
{
	bool active = false;
	Vector3 pos{ 0.0f, 0.0f, 0.0f };
	Vector3 dir{ 0.0f, 0.0f, 1.0f };
	Vector3 color{ 1.0f, 1.0f, 1.0f };

	float speed = 20.0f;
	float range = 10.0f;
	float intensity = 2.5f;

	float life = 0.0f;
	float maxLife = 2.0f;
};

//struct ShadowData
//{
//	Matrix lightViewProj;
//};