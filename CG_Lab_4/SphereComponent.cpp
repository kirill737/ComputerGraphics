#include "SphereComponent.h"
#include "Camera.h"
#include <d3dcompiler.h>
#include <cmath>

#include "WICTextureLoader.h"
#pragma comment(lib, "Windowscodecs.lib")

namespace CGLib {

	bool SphereComponent::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd)
	{
		context_ = context;

		ID3DBlob* vsBlob = nullptr;
		if (!CompileShader(L"./Shaders/MyVeryFirstShader.hlsl", "VSMain", "vs_5_0", &vsBlob, nullptr)) return false;
		if (FAILED(device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader_))) return false;

		ID3DBlob* psBlob = nullptr;
		if (!CompileShader(L"./Shaders/MyVeryFirstShader.hlsl", "PSMain", "ps_5_0", &psBlob, nullptr)) {
			vsBlob->Release(); return false;
		}

		if (FAILED(device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader_))) {
			vsBlob->Release(); psBlob->Release(); return false;
		}

		D3D11_INPUT_ELEMENT_DESC layout[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		if (FAILED(device->CreateInputLayout(
			layout,
			4,
			vsBlob->GetBufferPointer(),
			vsBlob->GetBufferSize(),
			&inputLayout_))) {
			vsBlob->Release(); psBlob->Release(); return false;
		}

		vsBlob->Release(); psBlob->Release();

		std::vector<Vertex> vertices;
		std::vector<UINT> indices;

		// Северный полюс
		vertices.push_back({
			{ 0.0f, radius_, 0.0f },
			color_,
			{ 0.5f, 0.0f },
			{ 0.0f, 1.0f, 0.0f }
		});

		float phiStep = DirectX::XM_PI / stackCount_;
		float thetaStep = 2.0f * DirectX::XM_PI / sliceCount_;

		for (int i = 1; i <= stackCount_ - 1; i++)
		{
			float phi = i * phiStep;

			for (int j = 0; j <= sliceCount_; j++)
			{
				float theta = j * thetaStep;

				float x = radius_ * sinf(phi) * cosf(theta);
				float y = radius_ * cosf(phi);
				float z = radius_ * sinf(phi) * sinf(theta);

				float u = theta / (2.0f * DirectX::XM_PI);
				float v = phi / DirectX::XM_PI;

				Vector3 normal = Vector3(x, y, z);
				normal.Normalize();

				vertices.push_back({
					{ x, y, z },
					color_,
					{ u, v },
					normal
					});
			}
		}

		// Южный полюс
		vertices.push_back({
			{ 0.0f, -radius_, 0.0f },
			color_,
			{ 0.5f, 1.0f },
			{ 0.0f, -1.0f, 0.0f }
		});

	
		for (int i = 1; i <= sliceCount_; i++)
		{
			indices.push_back(0);
			indices.push_back(i + 1);
			indices.push_back(i);
		}

		int baseIndex = 1;
		int ringVertexCount = sliceCount_ + 1;

	
		for (int i = 0; i < stackCount_ - 2; i++)
		{
			for (int j = 0; j < sliceCount_; j++)
			{
				indices.push_back(baseIndex + i * ringVertexCount + j);
				indices.push_back(baseIndex + i * ringVertexCount + j + 1);
				indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

				indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
				indices.push_back(baseIndex + i * ringVertexCount + j + 1);
				indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
			}
		}

		
		int southPoleIndex = static_cast<int>(vertices.size() - 1);
		baseIndex = southPoleIndex - ringVertexCount;

		for (int i = 0; i < sliceCount_; i++)
		{
			indices.push_back(southPoleIndex);
			indices.push_back(baseIndex + i);
			indices.push_back(baseIndex + i + 1);
		}

		indexCount_ = static_cast<UINT>(indices.size());

		D3D11_BUFFER_DESC vbDesc = {};
		vbDesc.Usage = D3D11_USAGE_DEFAULT;
		vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbDesc.ByteWidth = sizeof(Vertex) * static_cast<UINT>(vertices.size());

		D3D11_SUBRESOURCE_DATA vbData = { vertices.data(), 0, 0 };
		if (FAILED(device->CreateBuffer(&vbDesc, &vbData, &vertexBuffer_))) return false;

		D3D11_BUFFER_DESC ibDesc = {};
		ibDesc.Usage = D3D11_USAGE_DEFAULT;
		ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibDesc.ByteWidth = sizeof(UINT) * static_cast<UINT>(indices.size());

		D3D11_SUBRESOURCE_DATA ibData = { indices.data(), 0, 0 };
		if (FAILED(device->CreateBuffer(&ibDesc, &ibData, &indexBuffer_))) return false;

		CD3D11_RASTERIZER_DESC rsDesc(D3D11_DEFAULT);
		rsDesc.CullMode = D3D11_CULL_NONE;
		//rsDesc.FillMode = D3D11_FILL_WIREFRAME;
		if (FAILED(device->CreateRasterizerState(&rsDesc, &rasterizerState_))) return false;

		D3D11_BUFFER_DESC cbDesc = {};
		cbDesc.ByteWidth = sizeof(TransformData);
		cbDesc.Usage = D3D11_USAGE_DEFAULT;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		if (FAILED(device->CreateBuffer(&cbDesc, nullptr, &transformBuffer_)))
			return false;

		if (!InitializeMaterial(device))
			return false;

		return true;
	}

	void SphereComponent::Render(ID3D11DeviceContext* context, const Camera& camera)
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
		SendMaterial(context);

		context->DrawIndexed(indexCount_, 0, 0);
	}

	void SphereComponent::Shutdown()
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