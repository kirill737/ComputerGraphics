#pragma once

#include "GameComponent.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <DirectXCollision.h>

namespace CGLib {

	class RacketComponent : public GameComponent
	{
	public:
		
		bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd) override;
		void Render(ID3D11DeviceContext* context) override;
		void Update(float deltaTime) override;
		void Shutdown() override;

		void GivePlayerControll() { isUnderPlayerControl_ = true; };
		void TakePlayerControll() { isUnderPlayerControl_ = false; }; // Вроде не понадобится

		float GetX() const { return pos_.x; };
		float GetY() const { return pos_.y; };
		float GetHeight() const { return height_; };
		float GetWidth() const { return width_; };
		float GetSpeed() const { return speed_; };


		DirectX::BoundingBox GetCollisionBox() const { return racketBox_; };


	private:


		Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer_;
		Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer_;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState_;

		//ID3D11DeviceContext* context_ = nullptr;
		UINT stride_ = 32; // Сколько байт занимает одна вершина
		UINT offset_ = 0;
		UINT indexCount_ = 6;

		// Параметры для перемещения
		
		float width_ = PXL;
		float height_ = 2.0f * PXL * 7.0f;
		float speed_ = 1.5f;

		DirectX::BoundingBox racketBox_;

		bool isUnderPlayerControl_ = false;

		
		

		/*void Up();
		void Down();*/
	};

}
