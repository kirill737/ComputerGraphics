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
		SphereComponent(float radius, int sliceCount = 16, int stackCount = 16)
			: radius_(radius), sliceCount_(sliceCount), stackCount_(stackCount) {
		}

		bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd) override;
		void Render(ID3D11DeviceContext* context, const Camera& camera) override;
		void Shutdown() override;

		virtual void Update(float deltaTime) override
		{
			if (selfRotationEnabled_)
			{
				selfRotationAngle_ += selfRotationSpeed_ * deltaTime;
				UpdateWorldMatrix();
			}
		}

		void SetSelfRotationSpeed(float speed)
		{
			selfRotationSpeed_ = speed;
		}

		void SetSelfRotationEnabled(bool enabled)
		{
			selfRotationEnabled_ = enabled;
		}

		void SetColor(const Vector4& color) { color_ = color; }

		float GetRadius() const { return radius_; }

	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer_;
		Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer_;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState_;

		struct Vertex
		{
			Vector3 position;
			Vector4 color;
			Vector2 tex;
			Vector3 normal;
		};

		UINT stride_ = sizeof(Vertex);
		UINT offset_ = 0;
		UINT indexCount_ = 0;

	protected:
		float radius_ = 1.0f;
		int sliceCount_ = 16;
		int stackCount_ = 16;

		bool selfRotationEnabled_ = true;
		Vector4 color_{ 1.0f, 1.0f, 1.0f, 1.0f };
	};
}