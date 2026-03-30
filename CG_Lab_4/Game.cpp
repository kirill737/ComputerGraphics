#include "Game.h"
#include "CubeComponent.h"
#include "SphereComponent.h"
#include "OrbitSphere.h"
#include "GroundComponent.h"
#include "BallComponent.h"

// Текстурки
#include "WICTextureLoader.h"
#include "ModelComponent.h"

#include <dxgi.h>
#include <chrono>
#include <iostream>
#include <WinUser.h>

#include <d3d.h>
#include <d3d11.h>
#include <random>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")
// Текстурки
#pragma comment(lib, "Windowscodecs.lib")

// Цвета главного шара
#define BALL_COLOR Vector4(226.0f / 256.0f, 192.0f / 256.0f, 68.0f / 256.0f, 1.0f)

// Вспомогательные функции
float RandomFloat(const float& start, const float& end)
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_real_distribution<float> dist(start, end);

	return dist(gen);
}

Vector3 RandomVector3(float min = 0.0f, float max = 1.0f) {
	return Vector3{ RandomFloat(min, max), RandomFloat(min, max), RandomFloat(min, max) };
}

Vector4 RandomColor() {
	return Vector4{ RandomFloat(0.0f, 1.0f), RandomFloat(0.0f, 1.0f), RandomFloat(0.0f, 1.0f), 1.0f };
}


namespace game {
    using namespace CGLib;
    using namespace DirectX::SimpleMath;


	bool Game::Initialize(HINSTANCE hInstance)
	{
		if (!display_.Initialize(L"My3DApp", screenWidth_, screenHeight_, hInstance, &input_))
			return false;

		if (!InitializeDirect3D())
			return false;

		camera_ = std::make_unique<CGLib::Camera>(screenWidth_, screenHeight_);
		camera_->SetPos(Vector3(0, 8, -12));
		camera_->LookAt(Vector3::Zero);



		std::string files[12] = {"apple", "banana", "blackberry", "coconut", "coconut_green", "lemon", "lime", "mango", "orange", "pear", "pineapple", "strawberry"};

		

		for (const auto& file : files) {
			auto model = std::make_shared<CGLib::ModelComponent>();

			model->SetColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
			Vector3 pos = RandomVector3(-20.0f, 20.0f);
			pos.y = 1.0f;
			model->SetPos(pos);

			float scale = 1.0f;
			model->SetScale(Vector3(scale, scale, scale));


			std::string objPath = "./Models/OBJ/Correct/" + file + ".fbx.obj";
			if (!model->LoadOBJ(objPath)) {
				std::cout << "Failed to load model: " << objPath << "\n";
				return false;
			}

			if (!model->Initialize(device_.Get(), context_.Get(), display_.GetHwnd())) {
				std::cout << "Failed to init model: " << file << "\n";
				return false;
			}

			std::wstring texturePath = L"./Textures/Textures/" +
				std::wstring(file.begin(), file.end()) + L".png";

			if (!model->LoadTexture(device_.Get(), context_.Get(), texturePath.c_str())) {
				std::cout << "Failed to load texture: " << file << "\n";
				return false;
			}
			worldObjects_.push_back(model);
			components_.push_back(model);
		}
		

		auto ground = std::make_shared<CGLib::GroundComponent>(100.0f, 100.0f);
		ground->SetColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		
		ground->SetPos(Vector3(0.0f, 0.0f, 0.0f));
		if (!ground->Initialize(device_.Get(), context_.Get(), display_.GetHwnd()))
			return false;
		if (!ground->LoadTexture(device_.Get(), context_.Get(), L"./Textures/mine_spruce.png"))
			return false;
		components_.push_back(ground);

		playerBall_ = std::make_shared<CGLib::BallComponent>(1.0f, 1024, 1024);
		playerBall_->SetColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		playerBall_->SetPos(Vector3(0.0f, 1.0f, 0.0f));
		playerBall_->SetMoveSpeed(30.0f);

		if (!playerBall_->Initialize(device_.Get(), context_.Get(), display_.GetHwnd()))
			return false;
		if (!playerBall_->LoadTexture(device_.Get(), context_.Get(), L"./Textures/WOOD.png"))
			return false;

		camera_->SetOrbitalTarget(playerBall_);
		components_.push_back(playerBall_);

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


		// Буфер глубины
		D3D11_TEXTURE2D_DESC depthDesc = {};
		depthDesc.Width = screenWidth_;
		depthDesc.Height = screenHeight_;
		depthDesc.MipLevels = 1;
		depthDesc.ArraySize = 1;
		depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthDesc.SampleDesc.Count = 1;
		depthDesc.SampleDesc.Quality = 0;
		depthDesc.Usage = D3D11_USAGE_DEFAULT;
		depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthDesc.CPUAccessFlags = 0;
		depthDesc.MiscFlags = 0;

		if (FAILED(device_->CreateTexture2D(&depthDesc, nullptr, &depthStencilBuffer_)))
			return false;

		if (FAILED(device_->CreateDepthStencilView(depthStencilBuffer_.Get(), nullptr, &depthStencilView_)))
			return false;


        return true;
    }

	void Game::UpdateKatamari()
	{
		if (!playerBall_) return;

		Vector3 ballPos = playerBall_->GetPos();
		float ballRadius = playerBall_->GetRadius() * playerBall_->GetScale().x;

		Matrix ballRotation = playerBall_->GetSelfRotationMatrix();

		for (auto& obj : worldObjects_)
		{
			if (!obj || obj->IsCollected())
				continue;

			Vector3 objPos = obj->GetPos();
			float objRadius = obj->GetBoundingRadius();

			float dist = (ballPos - objPos).Length();

			if (dist <= ballRadius + objRadius)
			{
				obj->SetCollected(true);

				Vector3 worldOffset = objPos - ballPos;

				Matrix invBallRotation = ballRotation.Invert();
				Vector3 localOffset = Vector3::Transform(worldOffset, invBallRotation);

				obj->SetAttachOffset(localOffset);
				obj->SetAttachRotationOffset(DirectX::SimpleMath::Matrix::Identity);

				attachedObjects_.push_back(obj);
			}
		}

		for (auto& obj : attachedObjects_)
		{
			if (!obj) continue;

			Vector3 rotatedOffset = Vector3::Transform(obj->GetAttachOffset(), ballRotation);
			obj->SetPos(playerBall_->GetPos() + rotatedOffset);

			obj->SetExternalRotation(ballRotation * obj->GetAttachRotationOffset());
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

			UpdatePlayer(deltaTime);
			for (auto& comp : components_) comp->Update(deltaTime);
			UpdateKatamari();
			UpdateFPS(deltaTime);
			UpdateCamera(deltaTime);
			RenderFrame();
        }
    }

	void Game::RenderFrame()
	{

		float bgColor[] = { 242.0f / 256.0f, 227.0f / 256.0f, 187.0f / 256.0f, 1.0f };

		context_->ClearState();
		context_->ClearRenderTargetView(renderTargetView_.Get(), bgColor);
		context_->ClearDepthStencilView(depthStencilView_.Get(),
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
			1.0f,
			0);

		D3D11_VIEWPORT vp = {
			0, 0,
			static_cast<float>(screenWidth_),
			static_cast<float>(screenHeight_),
			0.0f, 1.0f
		};

		context_->RSSetViewports(1, &vp);
		context_->OMSetRenderTargets(1, renderTargetView_.GetAddressOf(), depthStencilView_.Get());

		for (auto& comp : components_) {
			comp->Render(context_.Get(), *camera_);
		}

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
		/*if (input_.IsKeyPressed('W')) camera_->MoveForward(deltaTime, speed);
		if (input_.IsKeyPressed('S')) camera_->MoveBackward(deltaTime, speed);
		if (input_.IsKeyPressed('A')) camera_->MoveLeft(deltaTime, speed);
		if (input_.IsKeyPressed('D')) camera_->MoveRight(deltaTime, speed);*/
		if (input_.IsKeyPressed(VK_SPACE)) camera_->MoveUp(deltaTime, speed);
		if (input_.IsKeyPressed(VK_SHIFT)) camera_->MoveDown(deltaTime, speed);

		// Мышка
		POINT mouseDelta = input_.GetMouseDelta();
		camera_->Rotate(-mouseDelta.x * sensitivity, -mouseDelta.y * sensitivity);

		// Колесо мыши
		int wheelDelta = input_.GetWheelDelta();
		if (wheelDelta != 0 && camera_->GetMode() == CameraMode::Orbit)
		{
			camera_->Zoom(-wheelDelta * 0.01f);
		}

		if (camera_->GetMode() == CameraMode::Orbit)
		{
			camera_->UpdateOrbit();
		}
	}

	void Game::UpdatePlayer(float deltaTime)
	{
		if (!playerBall_ || !camera_) return;

		Vector3 forward = camera_->GetFlatForward();
		Vector3 right = camera_->GetFlatRight();

		Vector3 inputDir = Vector3::Zero;

		if (input_.IsKeyPressed('W')) inputDir += forward;
		if (input_.IsKeyPressed('S')) inputDir -= forward;
		if (input_.IsKeyPressed('D')) inputDir += right;
		if (input_.IsKeyPressed('A')) inputDir -= right;

		playerBall_->SetMoveInput(inputDir);
	}

    void Game::Shutdown()
    {
        for (auto& comp : components_) comp->Shutdown();
        components_.clear();

        renderTargetView_.Reset();
		depthStencilView_.Reset();
		depthStencilBuffer_.Reset();

        context_.Reset();
        device_.Reset();
        swapChain_.Reset();

        display_.Shutdown();
    }

}