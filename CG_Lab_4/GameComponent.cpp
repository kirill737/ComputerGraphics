#include "GameComponent.h"
#include <d3dcompiler.h>
#include <iostream>

#include "WICTextureLoader.h"
#pragma comment(lib, "Windowscodecs.lib")

namespace CGLib {

	bool GameComponent::Initialize(
		ID3D11Device* device,
		ID3D11DeviceContext* context,
		HWND hwnd)
	{
		device_ = device;
		context_ = context;

		return true;
	}

	bool GameComponent::CompileShader(
		const wchar_t* filename,
		const char* entryPoint,
		const char* target,
		ID3DBlob** blob,
		D3D_SHADER_MACRO* macros)
	{
		ID3DBlob* errorBlob = nullptr;

		HRESULT res = D3DCompileFromFile(
			filename,
			macros,
			nullptr,
			entryPoint,
			target,
			D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
			0,
			blob,
			&errorBlob);

		if (FAILED(res))
		{
			if (errorBlob)
			{
				std::cout << static_cast<char*>(errorBlob->GetBufferPointer()) << std::endl;
				errorBlob->Release();
			}
			else
			{
				std::wcout << L"Shader file not found: " << filename << std::endl;
			}

			return false;
		}

		return true;
	}

	bool GameComponent::InitializeTransform(ID3D11Device* device)
	{
		D3D11_BUFFER_DESC cbDesc = {};

		cbDesc.ByteWidth = sizeof(TransformData);
		cbDesc.Usage = D3D11_USAGE_DEFAULT;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		return !FAILED(device->CreateBuffer(
			&cbDesc,
			nullptr,
			&transformBuffer_));
	}

	void GameComponent::SendTransform(
		ID3D11DeviceContext* context,
		const Matrix& view,
		const Matrix& proj)
	{
		TransformData data;

		data.world = worldMatrix_.Transpose();
		data.view = view.Transpose();
		data.proj = proj.Transpose();

		context->UpdateSubresource(
			transformBuffer_.Get(),
			0,
			nullptr,
			&data,
			0,
			0);

		context->VSSetConstantBuffers(
			0,
			1,
			transformBuffer_.GetAddressOf());
	}

	bool CGLib::GameComponent::LoadTexture(
		ID3D11Device* device,
		ID3D11DeviceContext* context,
		const wchar_t* filename)
	{
		HRESULT hr = DirectX::CreateWICTextureFromFile(
			device,
			context,
			filename,
			nullptr,
			&texture_);

		if (FAILED(hr))
			return false;

		D3D11_SAMPLER_DESC sampDesc = {};
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

		if (FAILED(device->CreateSamplerState(&sampDesc, &samplerState_)))
			return false;

		return true;
	}

	void GameComponent::BindTexture(ID3D11DeviceContext* context)
	{
		if (texture_)
			context->PSSetShaderResources(0, 1, texture_.GetAddressOf());

		if (samplerState_)
			context->PSSetSamplers(0, 1, samplerState_.GetAddressOf());
		return;
	}

	void GameComponent::UpdateWorldMatrix()
	{
		Matrix scaleMatrix = Matrix::CreateScale(scale_);
		Matrix selfRotation = Matrix::CreateFromQuaternion(rotation_);
		Matrix translationMatrix = Matrix::CreateTranslation(pos_);

		worldMatrix_ =
			scaleMatrix *
			selfRotation *
			externalRotation_ *
			translationMatrix;
	}

	// Материал
	bool GameComponent::InitializeMaterial(ID3D11Device* device)
	{
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = sizeof(MaterialData);
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		return !FAILED(device->CreateBuffer(&desc, nullptr, &materialBuffer_));
	}

	void GameComponent::SendMaterial(ID3D11DeviceContext* context)
	{
		context->UpdateSubresource(materialBuffer_.Get(), 0, nullptr, &material_, 0, 0);
		context->PSSetConstantBuffers(2, 1, materialBuffer_.GetAddressOf());
	}

	// Тени
	bool GameComponent::InitializeShadowBuffer(ID3D11Device* device)
	{
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = sizeof(ShadowData);
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		return !FAILED(device->CreateBuffer(&desc, nullptr, &shadowBuffer_));
	}

	void GameComponent::SendShadowData(ID3D11DeviceContext* context, const Matrix& lightViewProj)
	{
		ShadowData data;
		data.lightViewProj = lightViewProj.Transpose();

		context->UpdateSubresource(shadowBuffer_.Get(), 0, nullptr, &data, 0, 0);
		context->VSSetConstantBuffers(3, 1, shadowBuffer_.GetAddressOf());
	}
}