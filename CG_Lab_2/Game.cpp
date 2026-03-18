#define NOMINMAX 1

#include "Game.h"
#include "TriangleComponent.h"
#include "RacketComponent.h"
#include "BallComponent.h"

#include <dxgi.h>
#include <chrono>
#include <iostream>

#include <windows.h>
#include <WinUser.h>
#include <wrl.h>
#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>


#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")


using CGLib::TriangleComponent;
using CGLib::RacketComponent;
namespace game {

    Game::Game() = default;
    Game::~Game() { Shutdown(); }

    bool Game::Initialize(HINSTANCE hInstance)
    {
        if (!display_.Initialize(L"My3DApp", screenWidth_, screenHeight_, hInstance, &input_))
            return false;

        if (!InitializeDirect3D()) return false;

        auto triangle = std::make_shared<TriangleComponent>();
        if (!triangle->Initialize(device_.Get(), context_.Get(), display_.GetHwnd())) {
            return false;
        }
        components_.push_back(triangle);

        // —оздаЄм спавнер
        spawner.Initialize(device_.Get(), context_.Get(), display_.GetHwnd());

		auto playerRacket = std::make_shared<CGLib::RacketComponent>();
		playerRacket->GivePlayerControll(&input_);
        playerRacket->SetPos(DirectX::SimpleMath::Vector2{ -0.9f, 0.0f });
		if (!playerRacket->Initialize(device_.Get(), context_.Get(), display_.GetHwnd()))
			return false;

		auto compRacket = std::make_shared<CGLib::RacketComponent>();
        compRacket->SetPos(DirectX::SimpleMath::Vector2{ 0.9f, 0.0f });
		if (!compRacket->Initialize(device_.Get(), context_.Get(), display_.GetHwnd()))
			return false;

		// ƒобавл€ем ракетки в Game
		components_.push_back(playerRacket);
		components_.push_back(compRacket);

		// ћ€ч
		auto ball = std::make_shared<CGLib::BallComponent>();
        ball->SetPos(DirectX::SimpleMath::Vector2{ 0.0f, 0.0f });

		// ѕередаем в м€ч weak_ptr на ракетки
		ball->AddRacket(playerRacket);
		ball->AddRacket(compRacket);
		if (!ball->Initialize(device_.Get(), context_.Get(), display_.GetHwnd()))
			return false;

		components_.push_back(ball);

        compRacket->SetBall(ball);

        spawner.AddRacket(playerRacket);

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

		for (auto& comp : components_) comp->Render(context_.Get());


		swapChain_->Present(1, 0);
	}

    void Game::UpdateFPS(float deltaTime)
    {
        totalTime_ += deltaTime;
        frameCount_++;

        spawner.UpdateTimer(deltaTime);

        if (totalTime_ >= 1.0f) {
            float fps = frameCount_ / totalTime_;
            wchar_t title[128];
            swprintf_s(title, L"My3DApp - FPS: %.1f", fps);
            SetWindowText(display_.GetHwnd(), title);

            totalTime_ = 0.0f;
            frameCount_ = 0;
        }
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