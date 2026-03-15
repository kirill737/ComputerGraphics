#pragma once

#include "GameComponent.h"
#include "RacketComponent.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <DirectXCollision.h>

#include <vector>
#include <memory>
#include <iostream>

namespace CGLib {

	class BallComponent : public GameComponent
	{
	public:

		bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd) override;
		void Render(ID3D11DeviceContext* context) override;
		void Update(float deltaTime) override;
		void Shutdown() override;

		// Отскоки мяча будут проверяться только со связанными с ним ракетками
		void AddRacket(std::shared_ptr<RacketComponent> racket);

		void AddPlayerScore() { playerScore++; };
		void AddCompScore() { compScore++; };
		short GetPlayerScore() const { return playerScore; };
		short GetCompScore() const { return compScore; };
		void ShowScore() const { std::cout << playerScore << "  |  " << compScore << std::endl; };
		void RespawnBall() { posX_ = 0.0f; posY_ = 0.0f; };


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
		float height_ = 2.0f * PXL;
		float speedX_ = 1.5f; //TODO: Подобрать скорость шарика
		float speedY_ = 3.0f;

		float minYSpeed = 0.3f;
		DirectX::BoundingBox ballBox;

		std::vector<std::weak_ptr<RacketComponent>> rackets_;

		// Счёт (лучше бы куда-то вынести в Game)
		unsigned short playerScore = 0;
		unsigned short compScore = 0;





		/*void Up();
		void Down();*/
	};

}
