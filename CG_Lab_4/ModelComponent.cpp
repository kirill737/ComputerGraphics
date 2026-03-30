#include "ModelComponent.h"
#include "Camera.h"
#include <d3dcompiler.h>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <tuple>
#include <cmath>

namespace CGLib
{
	namespace
	{
		struct ObjVertexKey
		{
			int v;
			int vt;

			bool operator==(const ObjVertexKey& other) const
			{
				return v == other.v && vt == other.vt;
			}
		};

		struct ObjVertexKeyHasher
		{
			size_t operator()(const ObjVertexKey& k) const
			{
				return (std::hash<int>()(k.v) * 73856093) ^ (std::hash<int>()(k.vt) * 19349663);
			}
		};
	}

	bool ModelComponent::LoadOBJ(const std::string& filename)
	{
		std::ifstream file(filename);
		if (!file.is_open())
			return false;

		std::vector<Vector3> positions;
		std::vector<Vector2> texcoords;

		std::unordered_map<ObjVertexKey, UINT, ObjVertexKeyHasher> uniqueVertices;

		std::string line;
		while (std::getline(file, line))
		{
			std::istringstream iss(line);
			std::string prefix;
			iss >> prefix;

			if (prefix == "v")
			{
				float x, y, z;
				iss >> x >> y >> z;
				positions.emplace_back(x, y, z);
			}
			else if (prefix == "vt")
			{
				float u, v;
				iss >> u >> v;
				texcoords.emplace_back(u, 1.0f - v);
			}
			else if (prefix == "f")
			{
				std::string tokens[4];
				int tokenCount = 0;

				while (tokenCount < 4 && (iss >> tokens[tokenCount]))
					tokenCount++;

				if (tokenCount < 3)
					continue;

				auto parseFaceVertex = [&](const std::string& token) -> UINT
					{
						std::istringstream tss(token);
						std::string a, b;

						std::getline(tss, a, '/');
						std::getline(tss, b, '/');

						int vi = std::stoi(a);
						int vti = b.empty() ? 0 : std::stoi(b);

						ObjVertexKey key{ vi, vti };

						auto it = uniqueVertices.find(key);
						if (it != uniqueVertices.end())
							return it->second;

						Vector3 p = positions[vi - 1];
						Vector2 uv = (vti > 0 && vti <= static_cast<int>(texcoords.size()))
							? texcoords[vti - 1]
							: Vector2(0.0f, 0.0f);

						Vertex vert;
						vert.position = DirectX::XMFLOAT4(p.x, p.y, p.z, 1.0f);
						vert.color = DirectX::XMFLOAT4(color_.x, color_.y, color_.z, color_.w);
						vert.tex = DirectX::XMFLOAT2(uv.x, uv.y);

						UINT newIndex = static_cast<UINT>(vertices_.size());
						vertices_.push_back(vert);
						uniqueVertices[key] = newIndex;
						return newIndex;
					};

				UINT i0 = parseFaceVertex(tokens[0]);
				UINT i1 = parseFaceVertex(tokens[1]);
				UINT i2 = parseFaceVertex(tokens[2]);

				indices_.push_back(i0);
				indices_.push_back(i1);
				indices_.push_back(i2);

				if (tokenCount == 4)
				{
					UINT i3 = parseFaceVertex(tokens[3]);

					indices_.push_back(i0);
					indices_.push_back(i2);
					indices_.push_back(i3);
				}
			}
		}

		indexCount_ = static_cast<UINT>(indices_.size());
		ComputeBoundingRadius();
		return !vertices_.empty() && !indices_.empty();
	}

	float ModelComponent::GetBoundingRadius() const { return boundingRadius_; }

	/*float ModelComponent::GetBoundingRadius() const
	{
		auto s = GetScale();
		float maxScale = std::max(s.x, std::max(s.y, s.z));
		return boundingRadius_ * maxScale;
	}*/

	void ModelComponent::ComputeBoundingRadius()
	{
		float maxDistSq = 0.0f;

		for (const auto& v : vertices_)
		{
			float x = v.position.x;
			float y = v.position.y;
			float z = v.position.z;
			float distSq = x * x + y * y + z * z;

			if (distSq > maxDistSq)
				maxDistSq = distSq;
		}

		boundingRadius_ = std::sqrt(maxDistSq);
	}

	bool ModelComponent::CreateBuffers(ID3D11Device* device)
	{
		D3D11_BUFFER_DESC vbDesc = {};
		vbDesc.Usage = D3D11_USAGE_DEFAULT;
		vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbDesc.ByteWidth = sizeof(Vertex) * static_cast<UINT>(vertices_.size());

		D3D11_SUBRESOURCE_DATA vbData = {};
		vbData.pSysMem = vertices_.data();

		if (FAILED(device->CreateBuffer(&vbDesc, &vbData, &vertexBuffer_)))
			return false;

		D3D11_BUFFER_DESC ibDesc = {};
		ibDesc.Usage = D3D11_USAGE_DEFAULT;
		ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibDesc.ByteWidth = sizeof(UINT) * static_cast<UINT>(indices_.size());

		D3D11_SUBRESOURCE_DATA ibData = {};
		ibData.pSysMem = indices_.data();

		if (FAILED(device->CreateBuffer(&ibDesc, &ibData, &indexBuffer_)))
			return false;

		return true;
	}

	bool ModelComponent::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd)
	{
		context_ = context;

		ID3DBlob* vsBlob = nullptr;
		if (!CompileShader(L"./Shaders/MyVeryFirstShader.hlsl", "VSMain", "vs_5_0", &vsBlob, nullptr))
			return false;

		if (FAILED(device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader_)))
			return false;

		ID3DBlob* psBlob = nullptr;
		if (!CompileShader(L"./Shaders/MyVeryFirstShader.hlsl", "PSMain", "ps_5_0", &psBlob, nullptr))
		{
			vsBlob->Release();
			return false;
		}

		if (FAILED(device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader_)))
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

		if (FAILED(device->CreateInputLayout(layout, 3, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout_)))
		{
			vsBlob->Release();
			psBlob->Release();
			return false;
		}

		vsBlob->Release();
		psBlob->Release();

		if (!CreateBuffers(device))
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

	void ModelComponent::Render(ID3D11DeviceContext* context, const Camera& camera)
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

	void ModelComponent::Shutdown()
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