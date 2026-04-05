#pragma once

#include "GameComponent.h"
#include <windows.h>
#include <d3d11.h>


struct LightBufferData
{
	DirectX::SimpleMath::Vector3 lightPos;
	float ambientStrength;

	DirectX::SimpleMath::Vector3 cameraPos;
	float specPower;

	DirectX::SimpleMath::Vector3 lightColor;
	float specStrength;
};