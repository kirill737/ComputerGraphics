#pragma once


#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl.h>
#include "SimpleMath.h"
#include "memory"



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
        virtual void OnCollision(std::shared_ptr<GameComponent> other) {};


		bool IsDestroyed() const { return destroyed_; }
		void Destroy() { destroyed_ = true; }



		bool CompileShader(const wchar_t* filename, const char* entryPoint,
			const char* target, ID3DBlob** blob, D3D_SHADER_MACRO* macros);


        bool InitializeTransform(ID3D11Device* device);
        void SendTransform(ID3D11DeviceContext* context);

        void SetPos(const DirectX::SimpleMath::Vector2& pos) {
            pos_ = pos;
			box_.Center = DirectX::XMFLOAT3(pos_.x, pos_.y, 0.0f);

			UpdateWorldMatrix();
        };
		DirectX::BoundingBox GetCollisionBox() const { return box_; };
        

		void Activate() { active_ = true; };
		void SetActive(bool value) { active_ = value; };
		bool IsActive() const { return active_; };

    protected:
        bool destroyed_ = false;
        ID3D11Device* device_ = nullptr;
        ID3D11DeviceContext* context_ = nullptr;

        DirectX::BoundingBox box_;
		/*DirectX::XMFLOAT2 position_ = { 0.0f, 0.0f };
		DirectX::XMFLOAT2 size_ = { 1.0f, 1.0f };*/


		/*float posX_ = 0.0;
		float posY_ = 0.0;*/
        DirectX::SimpleMath::Vector2 pos_{ 0.0f, 0.0f };

        Microsoft::WRL::ComPtr<ID3D11Buffer> transformBuffer_;
        DirectX::XMMATRIX worldMatrix_ = DirectX::XMMatrixIdentity(); // Матрица трансормаций

		void UpdateWorldMatrix()
		{
			worldMatrix_ = DirectX::XMMatrixTranslation(pos_.x, pos_.y, 0.0f);
		}


		
        bool active_ = true;
        
    };
}
