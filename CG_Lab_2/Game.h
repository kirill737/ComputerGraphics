#pragma once
#define NOMINMAX 1

#include "GameComponent.h"
#include "RacketComponent.h"
#include "BallComponent.h"
#include "DisplayWin32.h"
#include "InputDevice.h"
#include "Spawner.h"


#include <windows.h>
#include <d3d11.h>
#include <wrl.h>
#include <vector>
#include <memory>


using CGLib::GameComponent;
using CGLib::DisplayWin32;
using CGLib::InputDevice;

namespace game {

    class Game
    {
    public:
        Game();
        ~Game();

        bool Initialize(HINSTANCE hInstance);
        void Run();
        void Shutdown();

    private:
        Spawner spawner;

        bool InitializeDirect3D();
        void RenderFrame();
        void UpdateFPS(float deltaTime);

        DisplayWin32 display_;
        InputDevice input_;

        Microsoft::WRL::ComPtr<ID3D11Device> device_;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
        Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain_;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView_;

        std::vector<std::shared_ptr<GameComponent>> components_;

        int screenWidth_ = 800;
        int screenHeight_ = 400;

        // FPS counter
        float totalTime_ = 0.0f;
        unsigned int frameCount_ = 0;
    };
}