#pragma once

#include "GameComponent.h"
#include <d3d11.h>
#include <wrl.h>
#include <vector>

namespace CGLib
{
	using namespace DirectX::SimpleMath;




	class GroundComponent : public GameComponent
	{
	public:
		GroundComponent() = default;
		GroundComponent(float width, float depth)
			: width_(width), depth_(depth) {
		}

		bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd) override;
		void Render(ID3D11DeviceContext* context, const Camera& camera) override;
		void Shutdown() override;

		void SetColor(const Vector4& color) { color_ = color; }

		void RenderShadow(ID3D11DeviceContext* context, const Matrix& lightViewProj) override;
	private:
		struct Vertex
		{
			Vector3 position;
			Vector4 color;
			Vector2 tex;
			Vector3 normal;
		};

		float width_ = 20.0f;
		float depth_ = 20.0f;
		Vector4 color_{ 0.3f, 0.7f, 0.3f, 1.0f };

		UINT stride_ = sizeof(Vertex);
		UINT offset_ = 0;
		UINT indexCount_ = 0;

		Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer_;
		Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer_;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> shadowVertexShader_;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> shadowPixelShader_;

		Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState_;


	};
}