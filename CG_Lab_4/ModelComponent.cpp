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
			int vi;
			int vti;
			int vni;

			bool operator==(const ObjVertexKey& other) const
			{
				return vi == other.vi && vti == other.vti && vni == other.vni;
			}
		};

		struct ObjVertexKeyHasher
		{
			size_t operator()(const ObjVertexKey& k) const
			{
				size_t h1 = std::hash<int>{}(k.vi);
				size_t h2 = std::hash<int>{}(k.vti);
				size_t h3 = std::hash<int>{}(k.vni);

				return h1 ^ (h2 << 1) ^ (h3 << 2);
			}
		};
	}

	bool ModelComponent::LoadOBJ(const std::string& filename)
	{
		std::ifstream file(filename);
		if (!file.is_open())
			return false;

		vertices_.clear();
		indices_.clear();

		std::vector<Vector3> positions;
		std::vector<Vector2> texcoords;
		std::vector<Vector3> normals;

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
			else if (prefix == "vn")
			{
				float x, y, z;
				iss >> x >> y >> z;
				Vector3 n(x, y, z);
				n.Normalize();
				normals.push_back(n);
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
						std::string a, b, c;

						std::getline(tss, a, '/'); // v
						std::getline(tss, b, '/'); // vt
						std::getline(tss, c, '/'); // vn

						int vi = a.empty() ? 0 : std::stoi(a);
						int vti = b.empty() ? 0 : std::stoi(b);
						int vni = c.empty() ? 0 : std::stoi(c);

						ObjVertexKey key{ vi, vti, vni };

						auto it = uniqueVertices.find(key);
						if (it != uniqueVertices.end())
							return it->second;

						Vector3 p = (vi > 0 && vi <= (int)positions.size())
							? positions[vi - 1]
							: Vector3::Zero;

						Vector2 uv = (vti > 0 && vti <= (int)texcoords.size())
							? texcoords[vti - 1]
							: Vector2(0.0f, 0.0f);

						Vector3 n = (vni > 0 && vni <= (int)normals.size())
							? normals[vni - 1]
							: Vector3(0.0f, 1.0f, 0.0f);

						Vertex vert{};
						vert.position = p;
						vert.color = color_;
						vert.tex = uv;
						vert.normal = n;

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

		if (FAILED(device->CreateVertexShader(
			vsBlob->GetBufferPointer(),
			vsBlob->GetBufferSize(),
			nullptr,
			&vertexShader_)))
		{
			vsBlob->Release();
			return false;
		}

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

		ID3DBlob* shadowVsBlob = nullptr;
		if (!CompileShader(L"./Shaders/MyVeryFirstShader.hlsl", "VSShadow", "vs_5_0", &shadowVsBlob, nullptr))
		{
			vsBlob->Release();
			psBlob->Release();
			return false;
		}

		if (FAILED(device->CreateVertexShader(
			shadowVsBlob->GetBufferPointer(),
			shadowVsBlob->GetBufferSize(),
			nullptr,
			&shadowVertexShader_)))
		{
			vsBlob->Release();
			psBlob->Release();
			shadowVsBlob->Release();
			return false;
		}

		ID3DBlob* shadowPsBlob = nullptr;
		if (!CompileShader(L"./Shaders/MyVeryFirstShader.hlsl", "PSShadow", "ps_5_0", &shadowPsBlob, nullptr))
		{
			vsBlob->Release();
			psBlob->Release();
			shadowVsBlob->Release();
			return false;
		}

		if (FAILED(device->CreatePixelShader(
			shadowPsBlob->GetBufferPointer(),
			shadowPsBlob->GetBufferSize(),
			nullptr,
			&shadowPixelShader_)))
		{
			vsBlob->Release();
			psBlob->Release();
			shadowVsBlob->Release();
			shadowPsBlob->Release();
			return false;
		}

		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		if (FAILED(device->CreateInputLayout(
			layout,
			4,
			vsBlob->GetBufferPointer(),
			vsBlob->GetBufferSize(),
			&inputLayout_)))
		{
			vsBlob->Release();
			psBlob->Release();
			shadowVsBlob->Release();
			shadowPsBlob->Release();
			return false;
		}

		vsBlob->Release();
		psBlob->Release();
		shadowVsBlob->Release();
		shadowPsBlob->Release();

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

		if (!InitializeMaterial(device))
			return false;

		if (!InitializeShadowBuffer(device))
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
		SendMaterial(context);

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
	// Тени
	void ModelComponent::RenderShadow(ID3D11DeviceContext* context, const Matrix& lightViewProj)
	{
		context->RSSetState(rasterizerState_.Get());
		context->IASetInputLayout(inputLayout_.Get());
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);
		context->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride_, &offset_);

		context->VSSetShader(shadowVertexShader_.Get(), nullptr, 0);
		context->PSSetShader(shadowPixelShader_.Get(), nullptr, 0);

		SendTransform(context, Matrix::Identity, Matrix::Identity);
		SendShadowData(context, lightViewProj);

		context->DrawIndexed(indexCount_, 0, 0);
	}
}