#include "BallComponent.h"
#include "RacketComponent.h"
#include <iostream>
#include <cmath>
#include <random>

namespace CGLib {

	void BallComponent::OnCollision(std::shared_ptr<GameComponent> other) {
		if (auto racket = dynamic_cast<RacketComponent*>(other.get()))
		{
			std::cout << "Collision with racket!" << std::endl;

			// Настройка отскока
			float relativeIntersectY = (racket->GetY() - pos_.y);
			float normalizedRelativeIntersectionY = relativeIntersectY / (racket->GetHeight() / 2.0f);

			if (normalizedRelativeIntersectionY < -1.0f) normalizedRelativeIntersectionY = -1.0f;
			if (normalizedRelativeIntersectionY > 1.0f) normalizedRelativeIntersectionY = 1.0f;

			const float MAX_BOUNCE_ANGLE = 75.0f * 3.14159265f / 180.0f;
			float bounceAngle = normalizedRelativeIntersectionY * MAX_BOUNCE_ANGLE;

			float direction = (pos_.x < racket->GetX()) ? -1.0f : 1.0f;

			float currentSpeed = std::sqrt(speed_.x * speed_.x + speed_.y * speed_.y);
			currentSpeed += SpeedIncrement;

			speed_.x = currentSpeed * std::cos(bounceAngle) * direction;
			speed_.y = currentSpeed * -std::sin(bounceAngle);

			if (pos_.x < racket->GetX())
				pos_.x = racket->GetX() - racket->GetWidth() / 2 - width_ / 2;
			else
				pos_.x = racket->GetX() + racket->GetWidth() / 2 + width_ / 2;

			//std::cout << "X: " << speed_.x << " | Y: " << speed_.y << std::endl;
		}
	}

	bool BallComponent::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd)
	{
		context_ = context;

		// === ?????????? ???????? ===
		ID3DBlob* vsBlob = nullptr;
		if (!CompileShader(L"./Shaders/MyVeryFirstShader.hlsl", "VSMain", "vs_5_0", &vsBlob, nullptr)) return false;

		if (FAILED(device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
			nullptr, &vertexShader_))) return false;

		ID3DBlob* psBlob = nullptr;
		D3D_SHADER_MACRO macros[] = { {"TEST1", "0"}, {"TCOLOR", "float4(0.0f, 1.0f, 0.0f, 1.0f)"}, {nullptr, nullptr} };
		if (!CompileShader(L"./Shaders/MyVeryFirstShader.hlsl", "PSMain", "ps_5_0", &psBlob, macros)) {
			vsBlob->Release(); return false;
		}

		if (FAILED(device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(),
			nullptr, &pixelShader_))) {
			vsBlob->Release(); psBlob->Release(); return false;
		}

		// === Input Layout ===
		D3D11_INPUT_ELEMENT_DESC layout[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		if (FAILED(device->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(),
			vsBlob->GetBufferSize(), &inputLayout_))) {
			vsBlob->Release(); psBlob->Release(); return false;
		}
		vsBlob->Release(); psBlob->Release();

		// === Vert datas ===
		float halfWidth = width_ / 2.0f;
		float halfHeight = height_ / 2.0f;

		// TODO: Начальные корды через инициализацию?
		// { x, y, z (глубина?), нормаль }, { R, G, B, A } 
		DirectX::XMFLOAT4 vertices[] = {
			{ halfWidth,  halfHeight,  0.5f, 1.0f }, color_,
			{ -halfWidth, -halfHeight, 0.5f, 1.0f }, color_,
			{ halfWidth,  -halfHeight, 0.5f, 1.0f }, color_,
			{ -halfWidth,  halfHeight, 0.5f, 1.0f }, color_,
		};

		D3D11_BUFFER_DESC vbDesc = {};
		vbDesc.Usage = D3D11_USAGE_DEFAULT;
		vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbDesc.ByteWidth = sizeof(vertices);

		D3D11_SUBRESOURCE_DATA vbData = { vertices, 0, 0 };
		if (FAILED(device->CreateBuffer(&vbDesc, &vbData, &vertexBuffer_))) return false;


		//UINT indices[] = { 0,1,2};
		UINT indices[] = { 0,1,2, 1,0,3 };
		D3D11_BUFFER_DESC ibDesc = {};
		ibDesc.Usage = D3D11_USAGE_DEFAULT;
		ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibDesc.ByteWidth = sizeof(indices);

		D3D11_SUBRESOURCE_DATA ibData = { indices, 0, 0 };
		if (FAILED(device->CreateBuffer(&ibDesc, &ibData, &indexBuffer_))) return false;

		CD3D11_RASTERIZER_DESC rsDesc(D3D11_DEFAULT);
		rsDesc.CullMode = D3D11_CULL_NONE;
		//rsDesc.FillMode = D3D11_FILL_WIREFRAME; // Если нужны рамка
		if (FAILED(device->CreateRasterizerState(&rsDesc, &rasterizerState_))) return false;

		// Создаём буфер для перемещения ракетки

		D3D11_BUFFER_DESC cbDesc = {};
		cbDesc.ByteWidth = sizeof(DirectX::XMMATRIX);
		cbDesc.Usage = D3D11_USAGE_DEFAULT;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		if (FAILED(device->CreateBuffer(&cbDesc, nullptr, &transformBuffer_)))
			return false;

		return true;
	}

	void BallComponent::Render(ID3D11DeviceContext* context)
	{
		context->RSSetState(rasterizerState_.Get());
		context->IASetInputLayout(inputLayout_.Get());
		context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);
		context->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride_, &offset_);
		context->VSSetShader(vertexShader_.Get(), nullptr, 0);
		context->PSSetShader(pixelShader_.Get(), nullptr, 0);


		// Передаём матрицу в шейдер
		SendTransform(context);

		context->DrawIndexed(indexCount_, 0, 0);
	}

	void BallComponent::Update(float deltaTime)
	{
		if (!active_) return;

		// Двигаем мяч
		pos_.x += speed_.x * deltaTime;
		pos_.y += speed_.y * deltaTime;

		// Обновляем BoundingBox мяча
		box_.Center = DirectX::XMFLOAT3(pos_.x, pos_.y, 0.0f);
		box_.Extents = DirectX::XMFLOAT3(width_ / 2.0f, height_ / 2.0f, 0.1f);

		// Проверка границ экрана
		if (pos_.y + height_ / 2 > 1.0f || pos_.y - height_ / 2 < -1.0f) {
			speed_.y = -speed_.y; // отскок от верхней/нижней границы
		}

		// Левая стенка
		if (pos_.x - width_ / 2 < -1.0f) {
			speed_.x = -speed_.x;

			if (oneHit_) {
				SetActive(false);
				SetPos(DirectX::SimpleMath::Vector2{ 0.0f, 0.0f });
			}
			else 
				RespawnBall();
			AddCompScore();
			ShowScore();
		}

		// Правая стенка
		if (pos_.x + width_ / 2 > 1.0f) {
			speed_.x = -speed_.x;

			if (oneHit_) {
				SetActive(false);
				SetPos(DirectX::SimpleMath::Vector2{ 0.0f, 0.0f });
			}
			else
				RespawnBall();
			AddPlayerScore();
			ShowScore();
		}
		
		UpdateWorldMatrix();
	}

	void BallComponent::RespawnBall() {
		pos_ = DirectX::SimpleMath::Vector2{ 0.0f, 0.0f };
		speed_ = DirectX::SimpleMath::Vector2{ 1.5f, 0.0f };
	}

	float BallComponent::RandomFloat(float min, float max) {
		static std::random_device rd;
		static std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dist(min, max);
		return dist(gen);
	}

	void BallComponent::Shutdown()
	{
		vertexBuffer_.Reset();
		indexBuffer_.Reset();
		vertexShader_.Reset();
		pixelShader_.Reset();
		inputLayout_.Reset();
		rasterizerState_.Reset();
	}

}