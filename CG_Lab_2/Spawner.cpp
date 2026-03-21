#pragma once

#include "RacketComponent.h"
#include "DisplayWin32.h"
#include "InputDevice.h"
#include "ModificatorComponent.h"
#include "Spawner.h"

#include <windows.h>
#include <d3d11.h>
#include <wrl.h>
#include <vector>
#include <memory>

using CGLib::RacketComponent;
using CGLib::ModificatorComponent;

namespace game {

	void Spawner::Initialize(
		ID3D11Device* device,
		ID3D11DeviceContext* context,
		HWND hwnd,
		std::vector<std::shared_ptr<GameComponent>>* components
	)
	{
		device_ = device;
		context_ = context;
		hwnd_ = hwnd;
		components_ = components;
	}

	bool Spawner::Spawn()
	{
		auto mod = std::make_shared<CGLib::ModificatorComponent>();

		float x = rndFloat(L_BORDER, R_BORDER);
		float y = rndFloat(B_BORDER, T_BORDER);
		mod->SetPos({ x, y });

		if (!mod->Initialize(device_, context_, hwnd_))
			return false;

		components_->push_back(mod);

		return true;
	}

	void Spawner::UpdateTimer(float& deltaTime)
	{
		spawnTimeCounter += deltaTime;

		if (spawnTimeCounter >= spawnRate)
		{
			Spawn();
			spawnTimeCounter = 0.0f;
		}
	}
}