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
	bool Spawner::Spawn() {
		auto modif = std::make_shared<ModificatorComponent>();
		if (!modif->Initialize(device_.Get(), context_.Get(), display_.GetHwnd())) {
			return false;
		}
		
		return true;
	}

	void Spawner::UpdateTimer(float& deltatime) {
		spawnTimeCounter += deltatime;

		if (spawnTimeCounter >= spawnRate) {
			spawnTimeCounter = 0.0f;
			Spawn();
		}
	};
}