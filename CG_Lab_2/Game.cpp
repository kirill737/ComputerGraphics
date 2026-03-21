#define NOMINMAX 1

#include "Game.h"
#include "TriangleComponent.h"
#include "RacketComponent.h"
#include "BallComponent.h"

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

        // Перегородка по центру
        auto triangle = std::make_shared<TriangleComponent>();
        if (!triangle->Initialize(device_.Get(), context_.Get(), display_.GetHwnd())) {
            return false;
        }
        components_.push_back(triangle);

        // Создаём спавнер
        //auto spawner = std::make_unique<Spawner>();
		spawner_.Initialize(device_.Get(), context_.Get(), display_.GetHwnd(), &components_);

        // Ракетки игрока
		auto playerRacket = std::make_shared<CGLib::RacketComponent>();
		playerRacket->GivePlayerControll(&input_);
        playerRacket->SetPos(DirectX::SimpleMath::Vector2{ -0.9f, 0.0f });
		if (!playerRacket->Initialize(device_.Get(), context_.Get(), display_.GetHwnd()))
			return false;
        components_.push_back(playerRacket);

        // Ракетки компа
		auto compRacket = std::make_shared<CGLib::RacketComponent>();
        compRacket->SetPos(DirectX::SimpleMath::Vector2{ 0.9f, 0.0f });
		if (!compRacket->Initialize(device_.Get(), context_.Get(), display_.GetHwnd()))
			return false;
		components_.push_back(compRacket);

		// Мяч
		auto ball = std::make_shared<CGLib::BallComponent>();
        ball->SetPos(DirectX::SimpleMath::Vector2{ 0.0f, 0.0f });


		if (!ball->Initialize(device_.Get(), context_.Get(), display_.GetHwnd()))
			return false;
		components_.push_back(ball);

        compRacket->SetBall(ball);

		// Создаём пул шариков
		for (size_t i = 0; i < poolSize_; ++i) {
			auto ball = std::make_shared<CGLib::BallComponent>();
			ball->SetOneHit(true); // если нужно
			ball->SetActive(false); // начально не активен
			ball->SetColor(DirectX::SimpleMath::Vector4(0.34f, 0.5f, 0.5f, 1.0f));
			if (!ball->Initialize(device_.Get(), context_.Get(), display_.GetHwnd())) {
				std::cerr << "Failed to initialize pooled ball!" << std::endl;
				return false;
			}
			ballPool_.push_back(ball);
			components_.push_back(ball); // добавляем в общий вектор, чтобы рендерить/апдейтить
		}

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

    void Game::CheckCollisions() {
		for (auto& a : components_)
		{
			for (auto& b : components_)
			{
				if (a == b) continue;
				if (!a->IsActive() || !b->IsActive()) continue; // пропускаем неактивные объекты
				if (a->GetCollisionBox().Intersects(b->GetCollisionBox()))
				{
					a->OnCollision(b);
					b->OnCollision(a);
				}
			}
		}
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
            RemoveDestroyedComponents();
            ProcessPendingSpawns();
            UpdateFPS(deltaTime);
            CheckCollisions();
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
            if (!comp->IsActive()) {
                ccc++;
                continue; // пропускаем неактивные объекты
            }
			comp->Render(context_.Get());
		}

		//std::cout << "Active components: " << components_.size() - ccc << " / " << components_.size() << "\n";
			


		swapChain_->Present(1, 0);
	}

    void Game::UpdateFPS(float deltaTime)
    {
        totalTime_ += deltaTime;
        frameCount_++;

        spawner_.UpdateTimer(deltaTime);

        if (totalTime_ >= 1.0f) {
            float fps = frameCount_ / totalTime_;
            wchar_t title[128];
            swprintf_s(title, L"My3DApp - FPS: %.1f", fps);
            SetWindowText(display_.GetHwnd(), title);

            totalTime_ = 0.0f;
            frameCount_ = 0;
        }
    }


	void Game::SpawnBalls(const int& amount, const DirectX::SimpleMath::Vector2 pos)
	{
		std::cout << "Spawning " << amount << " balls at position: (" << pos.x << ", " << pos.y << ")\n";
		float angleStep = 30.0f; // угол между шарами в градусах
		float startAngle = -((amount - 1) * angleStep) / 2.0f;

		int spawned = 0;
		for (auto& ball : ballPool_) {
			if (!ball->IsActive() && spawned < amount) {
                ball->SetPos(DirectX::SimpleMath::Vector2{ pos.x + 0.1f, pos.y });

				// Рассчитываем направление под разным углом
				float angleRad = (startAngle + spawned * angleStep) * 3.14159265f / 180.0f;
				float speed = 3.0f;
				ball->SetSpeed(DirectX::SimpleMath::Vector2{ speed * std::cos(angleRad), speed * std::sin(angleRad) });

				ball->SetActive(true);
				ball->SetOneHit(true);

				spawned++;
			}
			if (spawned >= amount) break;
		}
	}


	void Game::RemoveDestroyedComponents() {
		components_.erase(
			std::remove_if(components_.begin(), components_.end(),
				[this](const std::shared_ptr<GameComponent>& obj) {
					if (obj->IsDestroyed()) {
						if (auto mod = std::dynamic_pointer_cast<CGLib::ModificatorComponent>(obj)) {
                            ballsToSpawn_.push_back(mod->GetPos());
						}
						return true;
					}
					return false;
				}),
			components_.end());
	}

	void Game::ProcessPendingSpawns() {
		for (auto& pos : ballsToSpawn_) {
			SpawnBalls(4, pos);
		}
		ballsToSpawn_.clear();
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