#define NOMINMAX 1

#include "Game.h"
#include "CubeComponent.h"
#include "SphereComponent.h"
#include "OrbitSphere.h"

#include <dxgi.h>
#include <chrono>
#include <iostream>
#include <WinUser.h>

#include <d3d.h>
#include <d3d11.h>


#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

// Цвета солнца, планет и лун
#define SUN_COLOR Vector4(226.0f / 256.0f, 192.0f / 256.0f, 68.0f / 256.0f, 1.0f)
#define VENUS_COLOR Vector4(241.0f / 256.0f, 179.0f / 256.0f, 65.0f / 256.0f, 1.0f)
#define MERCURY_COLOR Vector4(57.0f / 256.0f, 62.0f / 256.0f, 65.0f / 256.0f, 1.0f)
#define EARTH_COLOR Vector4(121.0f / 256.0f, 180.0f / 256.0f, 115.0f / 256.0f, 1.0f)
#define MARS_COLOR Vector4(226.0f / 256.0f, 118.0f / 256.0f, 68.0f / 256.0f, 1.0f)

#define MOON_COLOR Vector4(211.0f / 256.0f, 208.0f / 256.0f, 203.0f / 256.0f, 1.0f)


namespace game {
    using namespace CGLib;
    using namespace DirectX::SimpleMath;

    Game::Game() = default;
    Game::~Game() { Shutdown(); }

	// Центр, радиус орбиты, радиус сферы, цвет, центр орбиты, скорость орбиты
	std::shared_ptr<SphereComponent> Game::CreatePlanet(
		const Vector3& centerPos,
		float orbitRadius,
		float sphereRadius,
		const Vector4& color,
		std::shared_ptr<SphereComponent> orbitCenter,
		float orbitSpeed) // скорость вращения (радианы/сек)
	{
		std::shared_ptr<SphereComponent> planet;

		if (orbitCenter) {
			// Создаём орбитирующую сферу
			planet = std::make_shared<OrbitingSphere>(orbitCenter, orbitRadius, orbitSpeed, sphereRadius);
		}
		else {
			// Это центр (например, Солнце)
			planet = std::make_shared<SphereComponent>(centerPos, sphereRadius, 16, 16);
		}

		planet->SetColor(color);

		if (!planet->Initialize(device_.Get(), context_.Get(), display_.GetHwnd())) {
			throw std::runtime_error("Failed to initialize planet.");
		}

		components_.push_back(planet);
		return planet;
	}


    bool Game::Initialize(HINSTANCE hInstance)
    {
        if (!display_.Initialize(L"My3DApp", screenWidth_, screenHeight_, hInstance, &input_))
            return false;

        if (!InitializeDirect3D()) return false;

		camera_ = std::make_unique<CGLib::Camera>(screenWidth_, screenHeight_);
		camera_->SetPos(Vector3(0, 0, -5));
		camera_->LookAt(Vector3::Zero);


		/*auto cube = std::make_shared<CGLib::CubeComponent>(Vector3{ 0.0f, 0.0f, 0.0f }, 0.5f, 0.5f, 0.5f);
		if (!cube->Initialize(device_.Get(), context_.Get(), display_.GetHwnd())) {
			return false;
		}
		components_.push_back(cube);*/

		// Солнце
        auto sun = CreatePlanet({ 0,0,0 }, 0.0f, 3.0f, SUN_COLOR);

		// Меркурий
		auto mercury = CreatePlanet({}, 5.0f, 0.5f, MERCURY_COLOR, sun, 1.0f);

		// Венера
		auto venus = CreatePlanet({}, 7.0f, 0.6f, VENUS_COLOR, sun, 0.8f);
        //std::dynamic_pointer_cast<CGLib::OrbitingSphere>(venus)->SetOrbitAxis({ 0.0f, 0.0f, 1.0f });

		// Земля
		auto earth = CreatePlanet({}, 10.0f, 0.65f, EARTH_COLOR, sun, 0.6f);

		// Луна вокруг Земли
		auto moon = CreatePlanet({}, 1.0f, 0.2f, MOON_COLOR, earth, 2.0f); // маленький радиус, выше скорость
        





        return true;
    }

    bool Game::InitializeDirect3D()
    {
        D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;

        DXGI_SWAP_CHAIN_DESC swapDesc = {};
        swapDesc.BufferCount = 2;
        swapDesc.BufferDesc.Width = screenWidth_;
        swapDesc.BufferDesc.Height = screenHeight_;
        swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapDesc.BufferDesc.RefreshRate = { 60, 1 };
        swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapDesc.OutputWindow = display_.GetHwnd();
        swapDesc.Windowed = TRUE;
        swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapDesc.SampleDesc.Count = 1;

        HRESULT res = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
            0, &featureLevel, 1,
            D3D11_SDK_VERSION, &swapDesc, &swapChain_, &device_, nullptr, &context_);

        if (FAILED(res)) { std::cerr << "D3D11CreateDeviceAndSwapChain failed: " << res << std::endl; return false; }

        Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
        if (FAILED(swapChain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer)))) return false;
        if (FAILED(device_->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTargetView_))) return false;

        return true;
    }

   

    void Game::Run()
    {
        auto prevTime = std::chrono::steady_clock::now();

        std::cout << "display_.IsRunning(): " << display_.IsRunning() << "\n";

        while (display_.IsRunning())
        {
            if (!display_.ProcessMessages(&input_)) break;

            auto curTime = std::chrono::steady_clock::now();
            float deltaTime = std::chrono::duration<float>(curTime - prevTime).count();
            prevTime = curTime;

            for (auto& comp : components_) comp->Update(deltaTime);
            UpdateFPS(deltaTime);
            UpdateCamera(deltaTime);
            RenderFrame();
        }
    }

	void Game::RenderFrame()
	{
		float black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		context_->ClearState();
		context_->ClearRenderTargetView(renderTargetView_.Get(), black);

		D3D11_VIEWPORT vp = {
            0, 0,
            static_cast<float>(screenWidth_),
            static_cast<float>(screenHeight_),
            0.0f, 1.0f
        };
		context_->RSSetViewports(1, &vp);
		context_->OMSetRenderTargets(1, renderTargetView_.GetAddressOf(), nullptr);
        int ccc = 0;
		for (auto& comp : components_) {
			comp->Render(context_.Get(), *camera_);
		}

		//std::cout << "Active components: " << components_.size() - ccc << " / " << components_.size() << "\n
		swapChain_->Present(1, 0);
	}

    void Game::UpdateFPS(float deltaTime)
    {
        totalTime_ += deltaTime;
        frameCount_++;

     

        if (totalTime_ >= 1.0f) {
            float fps = frameCount_ / totalTime_;
            wchar_t title[128];
            swprintf_s(title, L"My3DApp - FPS: %.1f", fps);
            SetWindowText(display_.GetHwnd(), title);

            totalTime_ = 0.0f;
            frameCount_ = 0;
        }
    }

	void Game::UpdateCamera(float deltaTime)
	{
		const float speed = 5.0f;
		const float sensitivity = 0.0015f;

		// WASD
		if (input_.IsKeyPressed('W')) camera_->MoveForward(deltaTime, speed);
		if (input_.IsKeyPressed('S')) camera_->MoveBackward(deltaTime, speed);
		if (input_.IsKeyPressed('A')) camera_->MoveLeft(deltaTime, speed);
		if (input_.IsKeyPressed('D')) camera_->MoveRight(deltaTime, speed);
		if (input_.IsKeyPressed(VK_SPACE)) camera_->MoveUp(deltaTime, speed);
		if (input_.IsKeyPressed(VK_SHIFT)) camera_->MoveDown(deltaTime, speed);

		// Мышка
		POINT mouseDelta = input_.GetMouseDelta();
		camera_->Rotate(-mouseDelta.x * sensitivity, -mouseDelta.y * sensitivity);

		// Колесо мыши
		int wheelDelta = input_.GetWheelDelta(); // должен возвращать разницу прокрутки за кадр
		camera_->AdjustFOV(-wheelDelta * 0.001f); // масштабируем скорость изменения FOV
	}


    void Game::Shutdown()
    {
        for (auto& comp : components_) comp->Shutdown();
        components_.clear();

        renderTargetView_.Reset();
        context_.Reset();
        device_.Reset();
        swapChain_.Reset();

        display_.Shutdown();
    }

}