#pragma once

#include "GameComponent.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>

namespace CGLib 
{
	using namespace DirectX::SimpleMath;
    class CubeComponent : public GameComponent
    {
    public:
        CubeComponent() = default;
		CubeComponent(const Vector3& centre, const float& xExtent, const float& yExtent, const float& zExtent)
            : centre_(centre), xExtent_(xExtent), yExtent_(yExtent), zExtent_(zExtent) {}
        bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd) override;
        void Render(ID3D11DeviceContext* context, const Camera& camera) override;
        void Shutdown() override;

    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer_;
        Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer_;
        Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;
        Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState_;
	
		struct Vertex
		{
			DirectX::XMFLOAT4 position;
			DirectX::XMFLOAT4 color;
		};

        UINT stride_ = sizeof(Vertex);
        UINT offset_ = 0;
        UINT indexCount_ = 36;

		// Размеры куба
		Vector3 centre_{ 0.0f, 0.0f, 0.0f };
        float xExtent_ = 1.0f;
        float yExtent_ = 1.0f;
        float zExtent_ = 1.0f;

    };

}