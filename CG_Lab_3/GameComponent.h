#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>
#include "SimpleMath.h"
#include <memory>
#include "Camera.h"

namespace CGLib {

	constexpr float PXL = 0.03f;

	struct TransformData
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX proj;
	};

	class GameComponent
	{
	public:

		virtual ~GameComponent() = default;

		virtual bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd);

		virtual void Update(float deltaTime) {};
		virtual void Render(ID3D11DeviceContext* context, const Camera& camera) {};
		virtual void Shutdown() {};
		virtual void OnCollision(std::shared_ptr<GameComponent> other) {};

		bool CompileShader(const wchar_t* filename,
			const char* entryPoint,
			const char* target,
			ID3DBlob** blob,
			D3D_SHADER_MACRO* macros);

		bool InitializeTransform(ID3D11Device* device);

		void SendTransform(ID3D11DeviceContext* context, const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj);

		DirectX::SimpleMath::Vector3 GetPos() const { return pos_; };
		void SetPos(const DirectX::SimpleMath::Vector3& pos)
		{
			pos_ = pos;
			UpdateWorldMatrix();
		}

	protected:

		ID3D11Device* device_ = nullptr;
		ID3D11DeviceContext* context_ = nullptr;

		DirectX::SimpleMath::Vector3 pos_{ 0.0f,0.0f,0.0f };

		Microsoft::WRL::ComPtr<ID3D11Buffer> transformBuffer_;

		DirectX::XMMATRIX worldMatrix_ = DirectX::XMMatrixIdentity();

		void UpdateWorldMatrix()
		{
			worldMatrix_ = DirectX::XMMatrixTranslation(
				pos_.x,
				pos_.y,
				pos_.z);
		}

		bool active_ = true;

	};
}