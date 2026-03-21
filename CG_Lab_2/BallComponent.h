#pragma once

#include "GameComponent.h"

#include <d3d11.h>>
#include <memory>
#include <iostream>

namespace CGLib { class RacketComponent; }
namespace CGLib {

	class BallComponent : public GameComponent
	{
	public:

		bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd) override;
		void Render(ID3D11DeviceContext* context) override;
		void Update(float deltaTime) override;
		void Shutdown() override;

		// ќтскоки м€ча будут провер€тьс€ только со св€занными с ним ракетками
		void OnCollision(std::shared_ptr<GameComponent> other) override;
		float GetX() const { return pos_.x; };
		float GetY() const { return pos_.y; };


		// —чЄт
		void AddPlayerScore() { playerScore++; };
		void AddCompScore() { compScore++; };
		short GetPlayerScore() const { return playerScore; };
		short GetCompScore() const { return compScore; };
		void ShowScore() const { std::cout << playerScore << "  |  " << compScore << std::endl; };
		
		// Help
		float RandomFloat(float min, float max);
		

		void RespawnBall();
		void SetOneHit(bool value) { oneHit_ = value; };
		//void Activate() { active_ = true; };
		//void SetActive(bool value) { active_ = value; };
		//bool IsActive() const { return active_; };
		void SetYSpeed(float value) { speed_.y = value; };
		void SetSpeed(const DirectX::SimpleMath::Vector2& newSpeed) { speed_ = newSpeed; };
		void SetColor(const DirectX::SimpleMath::Vector4& newColor) { color_ = newColor; };

	private:


		Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer_;
		Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer_;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState_;


		UINT stride_ = 32; // —колько байт занимает одна вершина
		UINT offset_ = 0;
		UINT indexCount_ = 6;


		// –азмеры м€ча
		const float width_ = PXL;
		const float height_ = 2.0f * PXL;
		DirectX::SimpleMath::Vector4 color_{ 1.0f, 1.0f, 1.0f, 1.0f };

		// ѕеремещение м€ча
		const float SpeedIncrement = 0.05f;
		DirectX::SimpleMath::Vector2 speed_{ 1.5f, 1.5f };

		// ≈сли м€ч одноразовый
		bool oneHit_ = false;
		/*bool active_ = true;*/

		// —чЄт (лучше бы куда-то вынести в Game)
		unsigned int playerScore = 0;
		unsigned int compScore = 0;

	};

}
