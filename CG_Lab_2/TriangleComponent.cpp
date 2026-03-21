#include "TriangleComponent.h"
#include <d3dcompiler.h>
#include <iostream>


namespace CGLib {
    bool TriangleComponent::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd)
    {
        context_ = context;

        ID3DBlob* vsBlob = nullptr;
        if (!CompileShader(L"./Shaders/MyVeryFirstShader.hlsl", "VSMain", "vs_5_0", &vsBlob, nullptr)) return false;

        if (FAILED(device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
            nullptr, &vertexShader_))) return false;

        ID3DBlob* psBlob = nullptr;
        D3D_SHADER_MACRO macros[] = { {"TEST", "0"}, {"TCOLOR", "float4(0.0f, 1.0f, 0.0f, 1.0f)"}, {nullptr, nullptr} };
        if (!CompileShader(L"./Shaders/MyVeryFirstShader.hlsl", "PSMain", "ps_5_0", &psBlob, macros)) {
            vsBlob->Release(); return false;
        }

        if (FAILED(device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(),
            nullptr, &pixelShader_))) {
            vsBlob->Release(); psBlob->Release(); return false;
        }

        D3D11_INPUT_ELEMENT_DESC layout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        if (FAILED(device->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(),
            vsBlob->GetBufferSize(), &inputLayout_))) {
            vsBlob->Release(); psBlob->Release(); return false;
        }
        vsBlob->Release(); psBlob->Release();

        DirectX::XMFLOAT4 vertices[] = {
            { 0.01f,  0.9f, 0.0f, 1.0f }, { 0.8f, 0.8f, 0.8f, 1.0f },
            { -0.01f, -0.9f, 0.0f, 1.0f }, { 0.8f, 0.8f, 0.8f, 1.0f },
            { 0.01f, -0.9f, 0.0f, 1.0f }, { 0.8f, 0.8f, 0.8f, 1.0f },
            { -0.01f, 0.9f, 0.0f, 1.0f }, { 0.8f, 0.8f, 0.8f, 1.0f },
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
        //rsDesc.FillMode = D3D11_FILL_WIREFRAME; // Если нужна рамка
        if (FAILED(device->CreateRasterizerState(&rsDesc, &rasterizerState_))) return false;


		D3D11_BUFFER_DESC cbDesc = {};
		cbDesc.ByteWidth = sizeof(DirectX::XMMATRIX);
		cbDesc.Usage = D3D11_USAGE_DEFAULT;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		if (FAILED(device->CreateBuffer(&cbDesc, nullptr, &transformBuffer_)))
			return false;

        return true;
    }

    void TriangleComponent::Render(ID3D11DeviceContext* context)
    {
        context->RSSetState(rasterizerState_.Get());
        context->IASetInputLayout(inputLayout_.Get());
        context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);
        context->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride_, &offset_);
        context->VSSetShader(vertexShader_.Get(), nullptr, 0);
        context->PSSetShader(pixelShader_.Get(), nullptr, 0);

        SendTransform(context);

        context->DrawIndexed(indexCount_, 0, 0);
    }

    void TriangleComponent::Shutdown()
    {
        // ComPtr ????????? ??????? ?????????????, ?? ????? ???? ????????
        vertexBuffer_.Reset();
        indexBuffer_.Reset();
        vertexShader_.Reset();
        pixelShader_.Reset();
        inputLayout_.Reset();
        rasterizerState_.Reset();
    }

}