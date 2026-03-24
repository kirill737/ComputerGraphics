#pragma once
#define NOMINMAX 1

#include "GameComponent.h"
#include "DisplayWin32.h"
#include "InputDevice.h"
#include "Camera.h"
#include "SphereComponent.h"



#include <windows.h>
#include <d3d11.h>
#include <wrl.h>
#include <vector>
#include <memory>


//using CGLib::GameComponent;
//using CGLib::DisplayWin32;
//using CGLib::InputDevice;
using namespace CGLib;
namespace game {

    class Game
    {
    public:
        Game();
        ~Game();

        bool Initialize(HINSTANCE hInstance);
        void Run();
        void Shutdown();

		std::shared_ptr<SphereComponent> CreatePlanet(
			const Vector3& centerPos,
			float orbitRadius,
			float sphereRadius,
			const Vector4& color,
			std::shared_ptr<SphereComponent> orbitCenter = nullptr,
			float orbitSpeed = 0.0f,
			const Vector3& orbitAxis = Vector3::Up);

		void NextOrbitalTarget() {
			if (components_.empty()) return;

			currentOrbitalTarget = (currentOrbitalTarget + 1) % components_.size();
			camera_->SetOrbitalTarget(components_[currentOrbitalTarget]);
		}

		void PrevOrbitalTarget() {
			if (components_.empty()) return;

			currentOrbitalTarget =
				(currentOrbitalTarget + components_.size() - 1) % components_.size();
			camera_->SetOrbitalTarget(components_[currentOrbitalTarget]);
		}

    private:

        bool InitializeDirect3D();
        void RenderFrame();
        void UpdateFPS(float deltaTime);
        void UpdateCamera(float deltaTime);

        DisplayWin32 display_;
        InputDevice input_;
        std::unique_ptr<CGLib::Camera> camera_;

        Microsoft::WRL::ComPtr<ID3D11Device> device_;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
        Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain_;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView_;

        std::vector<std::shared_ptr<GameComponent>> components_;

        int screenWidth_ = 800;
        int screenHeight_ = 400;


        size_t currentOrbitalTarget = 0;
        // FPS counter
        float totalTime_ = 0.0f;
        unsigned int frameCount_ = 0;
    };
}