#pragma once

//#include "GameComponent.h"
#include "RacketComponent.h"
//#include "BallComponent.h"
//#include "DisplayWin32.h"
//#include "InputDevice.h"
#include "ModificatorComponent.h"
#include "InputDevice.h"

#include <windows.h>
#include <d3d11.h>
#include <wrl.h>
#include <vector>
#include <random>
#include <memory>

namespace game {

    class Spawner
    {
    public:
        Spawner();
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

        InputDevice* input_ = nullptr;
        
       
        float spawnTimeCounter = 0.0f;
        float spawnRate = 5.0f;

        // FPS counter
        float totalTime_ = 0.0f;
        unsigned int frameCount_ = 0;


        // Spawn box
        const float L_BORDER = -1.0f;
        const float R_BORDER = -0.5f;
        const float T_BORDER = 0.8f;
        const float B_BORDER = -0.8f;
    };
}