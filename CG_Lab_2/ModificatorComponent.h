#pragma once

#include "GameComponent.h"
#include <d3d11.h>
#include <DirectXCollision.h>
#include "SimpleMath.h"

namespace CGLib {
	class ModificatorComponent : public GameComponent
	{
	public:

		bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd) override;
		void Render(ID3D11DeviceContext* context) override;
		void Update(float deltaTime) override;
		void Shutdown() override;
		void OnCollision(std::shared_ptr<GameComponent> other) override;

		float GetX() const { return pos_.x; };
		float GetY() const { return pos_.y; };
		DirectX::SimpleMath::Vector2 GetPos() const { return pos_; };
		float GetHeight() const { return height_; };
		float GetWidth() const { return width_; };

		/*DirectX::SimpleMath::Vector2 GetRandomPos() {
			return DirectX::SimpleMath::Vector2{ rndFloat(L_BORDER, R_BORDER), rndFloat(B_BORDER, T_BORDER) };
		};*/


		//DirectX::BoundingBox GetCollisionBox() const { return box_; };


	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer_;
		Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer_;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState_;

		//InputDevice* input_;

		UINT stride_ = 32;
		UINT offset_ = 0;
		UINT indexCount_ = 6;

		// Размер
		float width_ = PXL * 4;
		float height_ = 2.0f * PXL * 4;
		//DirectX::BoundingBox box_;
		/*float speed_ = 1.5f;*/

	};

}
