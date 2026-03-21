#pragma once

#include "RacketComponent.h"
#include "ModificatorComponent.h"
#include "InputDevice.h"

#include <windows.h>
#include <d3d11.h>
#include <wrl.h>
#include <vector>
#include <random>
#include <memory>

using namespace CGLib;

namespace game {

    class Spawner
    {
    public:
        void Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd, std::vector<std::shared_ptr<GameComponent>>* components);
        // Random
        float rndFloat(float min, float max) {
            static std::mt19937 gen(std::random_device{}());
            std::uniform_real_distribution<float> dist(min, max);
            return dist(gen);
        }

        //bool Initialize();
        //void Run();
        //void Shutdown();
        bool Spawn();
        void UpdateTimer(float& deltatime);

    private:

        //InputDevice* input_ = nullptr;
		ID3D11Device* device_ = nullptr;
		ID3D11DeviceContext* context_ = nullptr;
		HWND hwnd_ = nullptr;
        
       
        float spawnTimeCounter = 0.0f;
        float spawnRate = 5.0f;

        // FPS counter
        float totalTime_ = 0.0f;
        unsigned int frameCount_ = 0;


		

		std::vector<std::shared_ptr<GameComponent>>* components_ = nullptr;

        // Spawn box
        const float L_BORDER = -1.0f;
        const float R_BORDER = -0.5f;
        const float T_BORDER = 0.8f;
        const float B_BORDER = -0.8f;
    };
}