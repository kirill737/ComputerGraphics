#pragma once


#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl.h>
#include "SimpleMath.h"



namespace CGLib {
    constexpr float PXL = 0.03f;
    
    class GameComponent
    {
    public:
        virtual ~GameComponent() = default;

        virtual bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd);
        //virtual bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd, D3D_SHADER_MACRO* macros = nullptr);


        virtual void Update(float deltaTime) {};

        virtual void Render(ID3D11DeviceContext* context) {};

        virtual void Shutdown() {};

		bool CompileShader(const wchar_t* filename, const char* entryPoint,
			const char* target, ID3DBlob** blob, D3D_SHADER_MACRO* macros);


        bool InitializeTransform(ID3D11Device* device);
        void SendTransform(ID3D11DeviceContext* context);

        void SetPos(const float& posX, const float& posY) {
            posX_ = posX;
            posY_ = posY;
        };

    protected:
        ID3D11Device* device_ = nullptr;
        ID3D11DeviceContext* context_ = nullptr;

		/*DirectX::XMFLOAT2 position_ = { 0.0f, 0.0f };
		DirectX::XMFLOAT2 size_ = { 1.0f, 1.0f };*/


		float posX_ = 0.0;
		float posY_ = 0.0;
        DirectX::SimpleMath::Vector2 pos;

        Microsoft::WRL::ComPtr<ID3D11Buffer> transformBuffer_;
        DirectX::XMMATRIX worldMatrix_ = DirectX::XMMatrixIdentity(); // Матрица трансормаций

		void UpdateWorldMatrix()
		{
			worldMatrix_ = DirectX::XMMatrixTranslation(posX_, posY_, 0.0f);
		}

        
    };
}
