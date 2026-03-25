#pragma once
#include "GameComponent.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <vector>

namespace CGLib {
	using namespace DirectX::SimpleMath;

	class SphereComponent : public GameComponent
	{
	public:
		SphereComponent() = default;
		SphereComponent(const Vector3& centre, float radius, int sliceCount = 16, int stackCount = 16)
			: centre_(centre), radius_(radius), sliceCount_(sliceCount), stackCount_(stackCount) {
		}

		bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd) override;
		void Render(ID3D11DeviceContext* context, const Camera& camera) override;
		void Shutdown() override;
		//void Update(float deltaTime) { rotationAngle_ += rotationSpeed_ * deltaTime; }
		virtual void Update(float deltaTime) override
		{
			selfRotationAngle_ += selfRotationSpeed_ * deltaTime;
			UpdateWorldMatrix();
		}

		void SetSelfRotationSpeed(float speed) { selfRotationSpeed_ = speed; }

		void SetSelfRotationAngle(float angle)
		{
			selfRotationAngle_ = angle;
			UpdateWorldMatrix();
		}
		void SetColor(const Vector4& color) { color_ = color; }

	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer_;
		Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer_;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState_;
		//Microsoft::WRL::ComPtr<ID3D11Buffer> transformBuffer_;

		struct Vertex
		{
			DirectX::XMFLOAT4 position;
			DirectX::XMFLOAT4 color;
		};

		UINT stride_ = sizeof(Vertex);
		UINT offset_ = 0;
		UINT indexCount_ = 0;

	protected:
		Vector3 centre_{ 0.0f, 0.0f, 0.0f };
		float radius_ = 1.0f;
		int sliceCount_ = 16;
		int stackCount_ = 16;

		//float rotationAngle_ = 0.0f;
		//float rotationSpeed_ = 1.0f;

		Vector4 color_{ 1.0f, 1.0f, 1.0f, 1.0f };
	};
}