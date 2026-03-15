#include "GameComponent.h"
#include <d3dcompiler.h>
#include <iostream>

namespace CGLib {
	bool GameComponent::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd) {
		device_ = device;
		context_ = context;
		return true;
	}

	bool GameComponent::CompileShader(const wchar_t* filename, const char* entryPoint,
		const char* target, ID3DBlob** blob, D3D_SHADER_MACRO* macros)
	{
		ID3DBlob* errorBlob = nullptr;
		HRESULT res = D3DCompileFromFile(filename, macros, nullptr, entryPoint, target,
			D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
			0, blob, &errorBlob);

		if (FAILED(res)) {
			if (errorBlob) {
				std::cout << static_cast<char*>(errorBlob->GetBufferPointer()) << std::endl;
				errorBlob->Release();
			}
			else {
				std::wcout << L"Shader file not found: " << filename << std::endl;
			}
			return false;
		}
		return true;
	}

	bool GameComponent::InitializeTransform(ID3D11Device* device)
	{
		D3D11_BUFFER_DESC cbDesc = {};
		cbDesc.ByteWidth = sizeof(DirectX::XMMATRIX);
		cbDesc.Usage = D3D11_USAGE_DEFAULT;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		return !FAILED(device->CreateBuffer(&cbDesc, nullptr, &transformBuffer_));
	}

	// Функция для передачи матрици трансормаций в шейдер
	void GameComponent::SendTransform(ID3D11DeviceContext* context)
	{
		// DirectXMath хранит матрицы "row-major"
		// HLSL шейдеры используют "column-major
		// Поэтому трансапанируем перед отправкой в шейдер
		auto matrix = DirectX::XMMatrixTranspose(worldMatrix_);

		context->UpdateSubresource(transformBuffer_.Get(), 0, nullptr, &matrix, 0, 0);
		context->VSSetConstantBuffers(0, 1, transformBuffer_.GetAddressOf());
	}
}