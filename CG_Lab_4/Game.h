#pragma once

#include "GameComponent.h"
#include "DisplayWin32.h"
#include "InputDevice.h"
#include "Camera.h"
#include "SphereComponent.h"
#include "BallComponent.h"
#include "ModelComponent.h"
#include "DirectLight.h"



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
		Game() = default;
		~Game() { Shutdown(); }
        bool Initialize(HINSTANCE hInstance);
        void Run();
        void Shutdown();

        void UpdatePlayer(float deltaTime);
        void UpdateKatamari();

        void UpdateLightBuffer();

        // Тени
        bool InitializeShadowMap();
        void UpdateLightMatrices();
        void UpdateShadowBuffer();
        void RenderShadowPass();
        Vector3 GetLightDir() const { return lightDir_; };
        void SetLightDir(const Vector3& newDir) { lightDir_ = newDir; };

        // Доп к 5 
		void UpdatePointLights(float deltaTime);
		void SpawnPointLight();

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

        int screenWidth_ = 2048;
        int screenHeight_ = 1152;


        size_t currentOrbitalTarget = 0;

        // Фэпасы
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
        Microsoft::WRL::ComPtr<ID3D11Buffer> shadowBuffer_;
        float lightAngle_ = 0.0f;
        Vector3 lightDir_ = Vector3(1.0f, -1.0f, 0.5f);

        // Тени
		Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowMapTexture_;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> shadowDSV_;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowSRV_;
        Microsoft::WRL::ComPtr<ID3D11SamplerState> shadowSamplerState_;

		D3D11_VIEWPORT shadowViewport_{};

		Matrix lightView_ = Matrix::Identity;
		Matrix lightProj_ = Matrix::Identity;
		Matrix lightViewProj_ = Matrix::Identity;

        // Доп на свет
		bool pointLightActive_ = false;
		Vector3 pointLightPos_{ 0.0f, 0.0f, 0.0f };
		Vector3 pointLightDir_{ 0.0f, 0.0f, 1.0f };
		Vector3 pointLightColor_{ 1.0f, 0.2f, 0.2f };

		float pointLightSpeed_ = 20.0f;
		float pointLightRange_ = 10.0f;
		float pointLightIntensity_ = 2.5f;
		float pointLightLife_ = 0.0f;
		float pointLightMaxLife_ = 2.0f;

		//static constexpr int MAX_PROJECTILE_LIGHTS = 8;
		std::vector<ProjectileLight> projectileLights_;

        // Доп 6
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowPatternSRV_;
    };
}