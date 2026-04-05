#pragma once

#include "GameComponent.h"
#include "DisplayWin32.h"
#include "InputDevice.h"
#include "Camera.h"
#include "SphereComponent.h"
#include "BallComponent.h"
#include "ModelComponent.h"



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

	struct LightBufferData
	{
		DirectX::SimpleMath::Vector3 lightPos;
		float ambientStrength;

		DirectX::SimpleMath::Vector3 cameraPos;
		float pad0 = 0.0f;

		DirectX::SimpleMath::Vector3 lightColor;
		float pad1 = 0.0f;
	};


    class Game
    {
    public:
		Game() = default;
		~Game() { Shutdown(); }
        bool Initialize(HINSTANCE hInstance);
        void Run();
        void Shutdown();

        void UpdatePlayer(float deltaTime);
        void UpdateKatamari();

        void UpdateLightBuffer();

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

        // Глубина
		Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer_;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView_;

        std::shared_ptr<CGLib::BallComponent> playerBall_;

		std::vector<std::shared_ptr<CGLib::ModelComponent>> worldObjects_;
		std::vector<std::shared_ptr<CGLib::ModelComponent>> attachedObjects_;

        // Свет
        Microsoft::WRL::ComPtr<ID3D11Buffer> lightBuffer_;
    };
}