#pragma once

#include "GameComponent.h"
#include <d3d11.h>
#include <wrl.h>
#include <vector>
#include <string>

namespace CGLib
{
	using namespace DirectX::SimpleMath;

	class ModelComponent : public GameComponent
	{
	public:
		ModelComponent() = default;

		bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd) override;
		void Render(ID3D11DeviceContext* context, const Camera& camera) override;
		void Shutdown() override;

		bool LoadOBJ(const std::string& filename);
		void SetColor(const Vector4& color) { color_ = color; }

		float GetBoundingRadius() const;

		bool IsCollected() const { return collected_; }
		void SetCollected(bool value) { collected_ = value; }

		void SetAttachOffset(const Vector3& offset) { attachOffset_ = offset; }
		const Vector3& GetAttachOffset() const { return attachOffset_; }

		void SetAttachRotationOffset(const DirectX::SimpleMath::Matrix& m) { attachRotationOffset_ = m; }

		const DirectX::SimpleMath::Matrix& GetAttachRotationOffset() const { return attachRotationOffset_; }

	private:
		struct Vertex
		{
			Vector3 position;
			Vector4 color;
			Vector2 tex;
			Vector3 normal;
		};

		bool CreateBuffers(ID3D11Device* device);
		void ComputeBoundingRadius();

		std::vector<Vertex> vertices_;
		std::vector<UINT> indices_;

		Vector4 color_{ 1.0f, 1.0f, 1.0f, 1.0f };
		float boundingRadius_ = 1.0f;

		UINT stride_ = sizeof(Vertex);
		UINT offset_ = 0;
		UINT indexCount_ = 0;

		Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer_;
		Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer_;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState_;

		bool collected_ = false;
		Vector3 attachOffset_{ 0.0f, 0.0f, 0.0f };

		Matrix attachRotationOffset_ = Matrix::Identity;
	};
}