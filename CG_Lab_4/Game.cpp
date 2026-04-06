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
#define LOG(x) std::cout << "[LOG] " << x << std::endl
#define ERR(x) std::cout << "[ERROR] " << x << std::endl
// Цвета главного шара
#define BALL_COLOR Vector4(226.0f / 256.0f, 192.0f / 256.0f, 68.0f / 256.0f, 1.0f)

// Вспомогательные функции
float RandomFloat(const float& start, const float& end)
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dist(start, end);

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
		LOG("Game::Initialize started");

		if (!display_.Initialize(L"My3DApp", screenWidth_, screenHeight_, hInstance, &input_))
		{
			ERR("Display initialization failed");
			return false;
		}
		LOG("Display initialized");

		if (!InitializeDirect3D())
		{
			ERR("Direct3D initialization failed");
			return false;
		}
		LOG("Direct3D initialized");

		if (!InitializeShadowMap())
		{
			ERR("Shadow maps initialization failed");
			return false;
		}
		LOG("Shadow maps initialized");
		
		projectileLights_.reserve(MAX_PROJECTILE_LIGHTS);


		camera_ = std::make_unique<CGLib::Camera>(float(screenWidth_), float(screenHeight_));
		camera_->SetPos(Vector3(0, 8, -12));
		camera_->LookAt(Vector3::Zero);
		LOG("Camera created");

		std::string files[12] = { "apple", "banana", "blackberry", "coconut", "coconut_green", "lemon", "lime", "mango", "orange", "pear", "pineapple", "strawberry" };

		for (const auto& file : files)
		{
			LOG("Loading model: " << file);

			auto model = std::make_shared<CGLib::ModelComponent>();

			model->SetColor(Vector4(1, 1, 1, 1));

			Vector3 pos = RandomVector3(-20.0f, 20.0f);
			pos.y = RandomFloat(1.0f, 5.0f);

			model->SetPos(pos);
			model->SetScale(Vector3(1, 1, 1));

			model->SetShininess(8.0f);
			model->SetSpecStrength(0.1f);
			model->SetSpecularColor({ 0.4f, 0.4f, 0.4f });

			std::string objPath = "./Models/OBJ/Correct/" + file + ".fbx.obj";

			LOG("LoadOBJ: " << objPath);

			if (!model->LoadOBJ(objPath))
			{
				ERR("Failed to load OBJ: " << objPath);
				return false;
			}

			LOG("OBJ loaded");

			if (!model->Initialize(device_.Get(), context_.Get(), display_.GetHwnd()))
			{
				ERR("Model Initialize failed: " << file);
				return false;
			}

			LOG("Model GPU initialization success");

			std::wstring texturePath =
				L"./Textures/Textures/" +
				std::wstring(file.begin(), file.end()) +
				L".png";

			LOG("Loading texture");

			if (!model->LoadTexture(device_.Get(), context_.Get(), texturePath.c_str()))
			{
				ERR("Texture load failed: " << file);
				return false;
			}

			LOG("Texture loaded");

			worldObjects_.push_back(model);
			components_.push_back(model);
		}

		LOG("Models loaded");

		LOG("Creating ground");

		auto ground = std::make_shared<CGLib::GroundComponent>(100.0f, 100.0f);
		ground->SetColor(Vector4(1, 1, 1, 1));
		ground->SetPos(Vector3(0, 0, 0));
		//ground->SetSpecStrength(100.0f);
		ground->SetSpecularColor({ 1.0f, 1.0f, 1.0f });
		ground->SetSpecStrength(0.6f);
		ground->SetShininess(64.0f);

		if (!ground->Initialize(device_.Get(), context_.Get(), display_.GetHwnd()))
		{
			ERR("Ground initialization failed");
			return false;
		}

		LOG("Ground initialized");

		if (!ground->LoadTexture(device_.Get(), context_.Get(), L"./Textures/mine_spruce.png"))
		{
			ERR("Ground texture failed");
			return false;
		}

		LOG("Ground texture loaded");

		components_.push_back(ground);

		if (FAILED(DirectX::CreateWICTextureFromFile(
			device_.Get(),
			context_.Get(),
			L"./Textures/Smile.png",
			nullptr,
			&shadowPatternSRV_)))
		{
			ERR("Shadow pattern texture load failed");
			return false;
		}
		LOG("Shadow pattern texture loaded");


		LOG("Creating player ball");

		playerBall_ = std::make_shared<CGLib::BallComponent>(1.0f, 1024, 1024);

		playerBall_->SetColor(Vector4(1, 1, 1, 1));
		playerBall_->SetPos(Vector3(0, 1, 0));
		playerBall_->SetMoveSpeed(5.0f);

		playerBall_->SetShininess(20.0f);
		playerBall_->SetSpecStrength(1.0f);
		playerBall_->SetSpecularColor({ 1.0f, 1.0f, 1.0f });

		if (!playerBall_->Initialize(device_.Get(), context_.Get(), display_.GetHwnd()))
		{
			ERR("Ball initialization failed");
			return false;
		}

		LOG("Ball initialized");

		if (!playerBall_->LoadTexture(device_.Get(), context_.Get(), L"./Textures/WOOD.png"))
		{
			ERR("Ball texture failed");
			return false;
		}

		LOG("Ball texture loaded");

		camera_->SetOrbitalTarget(playerBall_);

		components_.push_back(playerBall_);

		LOG("Game initialization complete");

		return true;
	}

	bool Game::InitializeDirect3D()
	{
		LOG("Initializing Direct3D");

		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;

		DXGI_SWAP_CHAIN_DESC swapDesc = {};
		swapDesc.BufferCount = 2;
		swapDesc.BufferDesc.Width = screenWidth_;
		swapDesc.BufferDesc.Height = screenHeight_;
		swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapDesc.BufferDesc.RefreshRate = { 60,1 };
		swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapDesc.OutputWindow = display_.GetHwnd();
		swapDesc.Windowed = TRUE;
		swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapDesc.SampleDesc.Count = 1;

		HRESULT res = D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			0,
			&featureLevel,
			1,
			D3D11_SDK_VERSION,
			&swapDesc,
			&swapChain_,
			&device_,
			nullptr,
			&context_);

		if (FAILED(res))
		{
			ERR("D3D11CreateDeviceAndSwapChain failed");
			return false;
		}

		LOG("Device created");

		Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;

		if (FAILED(swapChain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer))))
		{
			ERR("SwapChain GetBuffer failed");
			return false;
		}

		if (FAILED(device_->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTargetView_)))
		{
			ERR("CreateRenderTargetView failed");
			return false;
		}

		LOG("RenderTargetView created");

		D3D11_TEXTURE2D_DESC depthDesc = {};

		depthDesc.Width = screenWidth_;
		depthDesc.Height = screenHeight_;
		depthDesc.MipLevels = 1;
		depthDesc.ArraySize = 1;
		depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthDesc.SampleDesc.Count = 1;
		depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		if (FAILED(device_->CreateTexture2D(&depthDesc, nullptr, &depthStencilBuffer_)))
		{
			ERR("Depth buffer creation failed");
			return false;
		}

		if (FAILED(device_->CreateDepthStencilView(depthStencilBuffer_.Get(), nullptr, &depthStencilView_)))
		{
			ERR("DepthStencilView creation failed");
			return false;
		}

		LOG("Depth buffer created");

		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = sizeof(LightBufferData);
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		if (FAILED(device_->CreateBuffer(&desc, nullptr, &lightBuffer_)))
		{
			ERR("Light buffer creation failed");
			return false;
		}

		LOG("Light buffer created");


		D3D11_SAMPLER_DESC shadowSampDesc = {};
		shadowSampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		shadowSampDesc.BorderColor[0] = 1.0f;
		shadowSampDesc.BorderColor[1] = 1.0f;
		shadowSampDesc.BorderColor[2] = 1.0f;
		shadowSampDesc.BorderColor[3] = 1.0f;
		shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		shadowSampDesc.MinLOD = 0;
		shadowSampDesc.MaxLOD = D3D11_FLOAT32_MAX;

		if (FAILED(device_->CreateSamplerState(&shadowSampDesc, &shadowSamplerState_)))
		{
			ERR("Shadow sampler creation failed");
			return false;
		}
		LOG("Shasow sampler created");


		D3D11_BUFFER_DESC shadowCbDesc = {};
		shadowCbDesc.ByteWidth = sizeof(ShadowData);
		shadowCbDesc.Usage = D3D11_USAGE_DEFAULT;
		shadowCbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		if (FAILED(device_->CreateBuffer(&shadowCbDesc, nullptr, &shadowBuffer_)))
		{
			ERR("Shadow buffer creation failed");
			return false;
		}
		LOG("Shadow buffer created");

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

			UpdatePlayer(deltaTime);
			UpdatePointLights(deltaTime);
			for (auto& comp : components_) comp->Update(deltaTime);
			UpdateKatamari();
			UpdateFPS(deltaTime);
			UpdateCamera(deltaTime);
			RenderFrame();
        }
    }

	void Game::RenderFrame()
	{
		UpdateLightMatrices();
		RenderShadowPass();

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

		UpdateLightBuffer();
		UpdateShadowBuffer();

		context_->PSSetShaderResources(1, 1, shadowSRV_.GetAddressOf());
		context_->PSSetShaderResources(2, 1, shadowPatternSRV_.GetAddressOf());
		context_->PSSetSamplers(1, 1, shadowSamplerState_.GetAddressOf());
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


	// Катамари
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


				Matrix objWorldRotation = obj->GetWorldRotationMatrix();
				Matrix localRotationOffset = objWorldRotation * invBallRotation;
				obj->SetAttachRotationOffset(localRotationOffset);

				attachedObjects_.push_back(obj);
			}
		}

		for (auto& obj : attachedObjects_)
		{
			if (!obj) continue;

			Vector3 rotatedOffset = Vector3::Transform(obj->GetAttachOffset(), ballRotation);
			obj->SetPos(playerBall_->GetPos() + rotatedOffset);

			Matrix finalRotation = obj->GetAttachRotationOffset() * ballRotation;
			obj->SetExternalRotation(finalRotation);
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

		static bool spacePressed = false;

		if (input_.IsKeyPressed(VK_SPACE))
		{
			if (!spacePressed)
			{
				playerBall_->RequestJump();
				spacePressed = true;
			}
		}
		else
		{
			spacePressed = false;
		}

		static bool shiftPressed = false;

		if (input_.IsKeyPressed(VK_SHIFT))
		{
			if (!shiftPressed)
			{
				playerBall_->SwitchNitro();
				shiftPressed = true;
			}
		}
		else
		{
			shiftPressed = false;
		}

		static bool ePressed = false;

		if (input_.IsKeyPressed('E'))
		{
			if (!ePressed)
			{
				SpawnPointLight();
				ePressed = true;
			}
		}
		else
		{
			ePressed = false;
		}
	}

	// Свет

	//void Game::UpdateLightBuffer()
	//{
	//	if (!camera_ || !context_ || !lightBuffer_)
	//		return;
	//
	//	LightBufferData lightData = {};
	//	//lightData.lightDir = Vector3(0.0f, -1.0f, 0.05f);
	//	lightData.lightDir = GetLightDir();
	//	lightData.lightDir.Normalize();
	//
	//	lightData.ambientStrength = 0.2f;
	//	lightData.cameraPos = camera_->GetPos();
	//	lightData.lightColor = Vector3(1.0f, 1.0f, 1.0f);
	//
	//	context_->UpdateSubresource(lightBuffer_.Get(), 0, nullptr, &lightData, 0, 0);
	//	context_->VSSetConstantBuffers(1, 1, lightBuffer_.GetAddressOf());
	//	context_->PSSetConstantBuffers(1, 1, lightBuffer_.GetAddressOf());
	//}

	void Game::UpdateLightBuffer()
	{
		if (!camera_ || !context_ || !lightBuffer_)
			return;

		LightBufferData lightData = {};
		lightData.lightDir = GetLightDir();
		lightData.lightDir.Normalize();

		lightData.ambientStrength = 0.2f;
		lightData.cameraPos = camera_->GetPos();
		lightData.lightColor = Vector3(1.0f, 1.0f, 1.0f);

		for (int i = 0; i < MAX_PROJECTILE_LIGHTS; i++)
		{
			lightData.pointLights[i].pos = Vector3(0.0f, 0.0f, 0.0f);
			lightData.pointLights[i].range = 0.0f;
			lightData.pointLights[i].color = Vector3(0.0f, 0.0f, 0.0f);
			lightData.pointLights[i].intensity = 0.0f;
		}

		int idx = 0;
		for (const auto& p : projectileLights_)
		{
			if (!p.active)
				continue;

			if (idx >= MAX_PROJECTILE_LIGHTS)
				break;

			lightData.pointLights[idx].pos = p.pos;
			lightData.pointLights[idx].range = p.range;
			lightData.pointLights[idx].color = p.color;
			lightData.pointLights[idx].intensity = p.intensity;
			idx++;
		}

		context_->UpdateSubresource(lightBuffer_.Get(), 0, nullptr, &lightData, 0, 0);
		context_->VSSetConstantBuffers(1, 1, lightBuffer_.GetAddressOf());
		context_->PSSetConstantBuffers(1, 1, lightBuffer_.GetAddressOf());
	}


	// Тени
	bool Game::InitializeShadowMap()
	{
		const UINT shadowSize = 2048*2;

		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = shadowSize;
		texDesc.Height = shadowSize;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		texDesc.SampleDesc.Count = 1;
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

		if (FAILED(device_->CreateTexture2D(&texDesc, nullptr, &shadowMapTexture_)))
			return false;

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

		if (FAILED(device_->CreateDepthStencilView(shadowMapTexture_.Get(), &dsvDesc, &shadowDSV_)))
			return false;

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		if (FAILED(device_->CreateShaderResourceView(shadowMapTexture_.Get(), &srvDesc, &shadowSRV_)))
			return false;

		shadowViewport_.TopLeftX = 0;
		shadowViewport_.TopLeftY = 0;
		shadowViewport_.Width = (float)shadowSize;
		shadowViewport_.Height = (float)shadowSize;
		shadowViewport_.MinDepth = 0.0f;
		shadowViewport_.MaxDepth = 1.0f;

		return true;
	}

	void Game::UpdateLightMatrices()
	{
		//Vector3 lightDir = Vector3(1.0f, -1.0f, 0.5f);
		Vector3 lightDir = GetLightDir();
		lightDir.Normalize();

		Vector3 sceneCenter = Vector3(0.0f, 0.0f, 0.0f);
		Vector3 lightPos = sceneCenter - lightDir * 50.0f;

		lightView_ = Matrix::CreateLookAt(lightPos, sceneCenter, Vector3::Up);
		lightProj_ = Matrix::CreateOrthographic(80.0f, 80.0f, 1.0f, 150.0f);

		lightViewProj_ = lightView_ * lightProj_;
	}

	void Game::UpdateShadowBuffer()
	{
		ShadowData data = {};
		data.lightViewProj = lightViewProj_.Transpose();

		context_->UpdateSubresource(shadowBuffer_.Get(), 0, nullptr, &data, 0, 0);
		context_->VSSetConstantBuffers(3, 1, shadowBuffer_.GetAddressOf());
		context_->PSSetConstantBuffers(3, 1, shadowBuffer_.GetAddressOf());
	}

	void Game::RenderShadowPass()
	{
		context_->OMSetRenderTargets(0, nullptr, shadowDSV_.Get());
		context_->RSSetViewports(1, &shadowViewport_);
		context_->ClearDepthStencilView(shadowDSV_.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

		for (auto& comp : components_)
		{
			comp->RenderShadow(context_.Get(), lightViewProj_);
		}
	}


	// Доп к 5
	void Game::SpawnPointLight()
	{
		if (!playerBall_ || !camera_)
			return;

		ProjectileLight p;
		p.active = true;
		p.pos = playerBall_->GetPos() + Vector3(0.0f, 1.0f, 0.0f);
		p.dir = camera_->GetFlatForward();

		if (p.dir.LengthSquared() < 0.0001f)
			p.dir = Vector3(0.0f, 0.0f, 1.0f);

		p.dir.Normalize();
		p.color = RandomVector3(0.2f, 1.0f);
		p.speed = 20.0f;
		p.range = 10.0f;
		p.intensity = 2.5f;
		p.life = 0.0f;
		p.maxLife = 3.0f;

		if ((int)projectileLights_.size() < MAX_PROJECTILE_LIGHTS)
		{
			projectileLights_.push_back(p);
		}
		else
		{
			projectileLights_[0] = p;
		}
	}

	void Game::UpdatePointLights(float deltaTime)
	{
		for (auto& p : projectileLights_)
		{
			if (!p.active)
				continue;

			p.pos += p.dir * p.speed * deltaTime;
			p.life += deltaTime;

			if (p.life >= p.maxLife)
				p.active = false;
		}

		projectileLights_.erase(
			std::remove_if(projectileLights_.begin(), projectileLights_.end(),
				[](const ProjectileLight& p) { return !p.active; }),
			projectileLights_.end()
		);
	}
}