#include "GroundComponent.h"
#include "Camera.h"
#include <d3dcompiler.h>

namespace CGLib
{
	bool GroundComponent::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd)
	{
		context_ = context;

		ID3DBlob* vsBlob = nullptr;
		if (!CompileShader(L"./Shaders/MyVeryFirstShader.hlsl", "VSMain", "vs_5_0", &vsBlob, nullptr))
			return false;

		if (FAILED(device->CreateVertexShader(
			vsBlob->GetBufferPointer(),
			vsBlob->GetBufferSize(),
			nullptr,
			&vertexShader_)))
			return false;

		ID3DBlob* psBlob = nullptr;
		if (!CompileShader(L"./Shaders/MyVeryFirstShader.hlsl", "PSMain", "ps_5_0", &psBlob, nullptr))
		{
			vsBlob->Release();
			return false;
		}

		if (FAILED(device->CreatePixelShader(
			psBlob->GetBufferPointer(),
			psBlob->GetBufferSize(),
			nullptr,
			&pixelShader_)))
		{
			vsBlob->Release();
			psBlob->Release();
			return false;
		}

		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		if (FAILED(device->CreateInputLayout(
			layout,
			3,
			vsBlob->GetBufferPointer(),
			vsBlob->GetBufferSize(),
			&inputLayout_)))
		{
			vsBlob->Release();
			psBlob->Release();
			return false;
		}

		vsBlob->Release();
		psBlob->Release();

		float halfWidth = width_ * 0.5f;
		float halfDepth = depth_ * 0.5f;

		std::vector<Vertex> vertices =
		{
			{{-halfWidth, 0.0f, -halfDepth, 1.0f}, color_, {0.0f, 0.0f}},
			{{ halfWidth, 0.0f, -halfDepth, 1.0f}, color_, {50.0f, 0.0f}},
			{{ halfWidth, 0.0f,  halfDepth, 1.0f}, color_, {50.0f, 50.0f}},
			{{-halfWidth, 0.0f,  halfDepth, 1.0f}, color_, {0.0f, 50.0f}}
		};

		std::vector<UINT> indices =
		{
			0, 1, 2,
			0, 2, 3
		};

		indexCount_ = static_cast<UINT>(indices.size());

		D3D11_BUFFER_DESC vbDesc = {};
		vbDesc.Usage = D3D11_USAGE_DEFAULT;
		vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbDesc.ByteWidth = sizeof(Vertex) * static_cast<UINT>(vertices.size());

		D3D11_SUBRESOURCE_DATA vbData = {};
		vbData.pSysMem = vertices.data();

		if (FAILED(device->CreateBuffer(&vbDesc, &vbData, &vertexBuffer_)))
			return false;

		D3D11_BUFFER_DESC ibDesc = {};
		ibDesc.Usage = D3D11_USAGE_DEFAULT;
		ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibDesc.ByteWidth = sizeof(UINT) * static_cast<UINT>(indices.size());

		D3D11_SUBRESOURCE_DATA ibData = {};
		ibData.pSysMem = indices.data();

		if (FAILED(device->CreateBuffer(&ibDesc, &ibData, &indexBuffer_)))
			return false;

		CD3D11_RASTERIZER_DESC rsDesc(D3D11_DEFAULT);
		rsDesc.CullMode = D3D11_CULL_NONE;
		rsDesc.FillMode = D3D11_FILL_SOLID;

		if (FAILED(device->CreateRasterizerState(&rsDesc, &rasterizerState_)))
			return false;

		D3D11_BUFFER_DESC cbDesc = {};
		cbDesc.ByteWidth = sizeof(TransformData);
		cbDesc.Usage = D3D11_USAGE_DEFAULT;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		if (FAILED(device->CreateBuffer(&cbDesc, nullptr, &transformBuffer_)))
			return false;

		UpdateWorldMatrix();
		return true;
	}

	void GroundComponent::Render(ID3D11DeviceContext* context, const Camera& camera)
	{
		context->RSSetState(rasterizerState_.Get());
		context->IASetInputLayout(inputLayout_.Get());
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);
		context->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride_, &offset_);
		context->VSSetShader(vertexShader_.Get(), nullptr, 0);
		context->PSSetShader(pixelShader_.Get(), nullptr, 0);

		BindTexture(context);
		SendTransform(context, camera.GetView(), camera.GetProjection());

		context->DrawIndexed(indexCount_, 0, 0);
	}

	void GroundComponent::Shutdown()
	{
		vertexBuffer_.Reset();
		indexBuffer_.Reset();
		vertexShader_.Reset();
		pixelShader_.Reset();
		inputLayout_.Reset();
		rasterizerState_.Reset();
		transformBuffer_.Reset();
	}
}