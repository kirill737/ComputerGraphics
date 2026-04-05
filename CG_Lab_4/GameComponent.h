#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>
#include "SimpleMath.h"
#include <memory>
#include "Camera.h"

namespace CGLib {
	using namespace DirectX::SimpleMath;
	constexpr float PXL = 0.03f;

	struct TransformData
	{
		Matrix world;
		Matrix view;
		Matrix proj;
	};

	struct MaterialData
	{
		DirectX::SimpleMath::Vector3 specularColor{ 1.0f, 1.0f, 1.0f };
		float shininess = 32.0f;

		float specStrength = 0.5f;
		DirectX::SimpleMath::Vector3 padding{ 0.0f, 0.0f, 0.0f };
	};

	struct ShadowData
	{
		Matrix lightViewProj;
	};

	class GameComponent
	{
	public:

		virtual ~GameComponent() = default;

		virtual bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd);

		virtual void Update(float deltaTime) {};
		virtual void Render(ID3D11DeviceContext* context, const Camera& camera) {};
		virtual void Shutdown() {};
		virtual void OnCollision(std::shared_ptr<GameComponent> other) {};

		bool CompileShader(const wchar_t* filename, const char* entryPoint, const char* target, ID3DBlob** blob, D3D_SHADER_MACRO* macros);

		bool InitializeTransform(ID3D11Device* device);

		void SendTransform(ID3D11DeviceContext* context, const Matrix& view, const Matrix& proj);

		Vector3 GetPos() const { return pos_; };
		void SetPos(const Vector3& pos)
		{
			pos_ = pos;
			UpdateWorldMatrix();
		}

		void SetSelfRotationAxis(const Vector3& axis)
		{
			selfRotationAxis_ = axis;
			selfRotationAxis_.Normalize();
			UpdateWorldMatrix();
		}

		// Для текстур
		bool LoadTexture(ID3D11Device* device, ID3D11DeviceContext* context, const wchar_t* filename);
		void BindTexture(ID3D11DeviceContext* context);

		void SetScale(const Vector3& scale)
		{
			scale_ = scale;
			UpdateWorldMatrix();
		}

		Vector3 GetScale() const { return scale_; }

		void SetExternalRotation(const Matrix& rotation)
		{
			externalRotation_ = rotation;
			UpdateWorldMatrix();
		}

		const Matrix& GetExternalRotation() const
		{
			return externalRotation_;
		}

		void SetRotation(const Quaternion& q)
		{
			rotation_ = q;
			rotation_.Normalize();
			UpdateWorldMatrix();
		}

		const Quaternion& GetRotation() const
		{
			return rotation_;
		}

		Matrix GetRotationMatrix() const
		{
			return Matrix::CreateFromQuaternion(rotation_);
		}

		Matrix GetSelfRotationMatrix() const
		{
			return Matrix::CreateFromQuaternion(rotation_);
		}

		Matrix GetWorldRotationMatrix() const
		{
			Matrix selfRotation = Matrix::CreateFromQuaternion(rotation_);
			return selfRotation * externalRotation_;
		}

		// Мтериал
		void SetSpecularColor(const Vector3& color) { material_.specularColor = color; }
		void SetShininess(float value) { material_.shininess = value; }
		void SetSpecStrength(float value) { material_.specStrength = value; }

		bool InitializeMaterial(ID3D11Device* device);
		void SendMaterial(ID3D11DeviceContext* context);

		// Тени
		virtual void RenderShadow(ID3D11DeviceContext* context, const Matrix& lightViewProj) {}
		bool InitializeShadowBuffer(ID3D11Device* device);
		void SendShadowData(ID3D11DeviceContext* context, const Matrix& lightViewProj);
	protected:

		ID3D11Device* device_ = nullptr;
		ID3D11DeviceContext* context_ = nullptr;

		Vector3 pos_{ 0.0f,0.0f,0.0f };

		Microsoft::WRL::ComPtr<ID3D11Buffer> transformBuffer_;

		Matrix worldMatrix_ = Matrix::Identity;

		// Материал
		MaterialData material_;
		Microsoft::WRL::ComPtr<ID3D11Buffer> materialBuffer_;
		
		//Тени
		Microsoft::WRL::ComPtr<ID3D11Buffer> shadowBuffer_;

		
		void UpdateWorldMatrix();

		float selfRotationAngle_ = 0.0f;
		float selfRotationSpeed_ = 0.0f;
		Vector3 selfRotationAxis_{ 0.0f, 1.0f, 0.0f };
		Quaternion rotation_ = Quaternion::Identity;

		bool active_ = true;

		// Для текстур
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture_;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState_;

		Vector3 scale_{ 1.0f, 1.0f, 1.0f };
		Matrix externalRotation_ = Matrix::Identity;

	};
}