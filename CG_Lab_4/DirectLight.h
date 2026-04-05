#pragma once

#include "GameComponent.h"
#include <windows.h>
#include <d3d11.h>


struct LightBufferData
{
	DirectX::SimpleMath::Vector3 lightDir;
	float ambientStrength;

	DirectX::SimpleMath::Vector3 cameraPos;
	float pad0 = 0.0f;

	DirectX::SimpleMath::Vector3 lightColor;
	float pad1 = 0.0f;
};

//struct ShadowData
//{
//	Matrix lightViewProj;
//};