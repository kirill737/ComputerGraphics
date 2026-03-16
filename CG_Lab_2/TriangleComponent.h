#pragma once

#include "GameComponent.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>

namespace CGLib {

    class TriangleComponent : public GameComponent
    {
    public:
        bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd) override;
        void Render(ID3D11DeviceContext* context) override;
        void Shutdown() override;

    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer_;
        Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer_;
        Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;
        Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState_;
	

        UINT stride_ = 32;
        UINT offset_ = 0;
        UINT indexCount_ = 6;
    };

}