#pragma once

#include "SimpleMath.h"
#include "InputDevice.h"
#include <iostream>

namespace CGLib
{
	using namespace DirectX::SimpleMath;

	class Camera final
	{
	public:

		Camera();
		Camera(const float& screenWidth, const float& screenHeight)
		{
			screenPropotion_ = screenWidth / screenHeight;
		};

		void SetPos(const Vector3& pos);
		void LookAt(const Vector3& target);

		const Matrix& GetView() const;
		const Matrix& GetProjection() const;

		void UpdateViewMatrix();

		// Движение камеры
		void MoveForward(float dt, float speed) { position_ += forward_ * speed * dt; }
		void MoveBackward(float dt, float speed) { position_ -= forward_ * speed * dt; }
		void MoveRight(float dt, float speed) { position_ += right_ * speed * dt; }
		void MoveLeft(float dt, float speed) { position_ -= right_ * speed * dt; }
		void MoveUp(float dt, float speed) { position_ += up_ * speed * dt; }
		void MoveDown(float dt, float speed) { position_ -= up_ * speed * dt; }

		void UpdateVectors();
		void Rotate(float deltaYaw, float deltaPitch);


		void AdjustFOV(float delta) {
			fov_ += delta;
			// Ограничиваем FOV, чтобы не было слишком маленьким или слишком большим
			if (fov_ < 0.1f) fov_ = 0.1f;
			if (fov_ > DirectX::XM_PIDIV2) fov_ = DirectX::XM_PIDIV2;
		}

	private:

		float screenPropotion_ = 1.0f;
		/*Vector3 position_;*/
		Vector3 target_;

		Matrix viewMatrix_;
		Matrix projectionMatrix_;
		InputDevice* input_ = nullptr;

		float fov_ = DirectX::XM_PIDIV4;
		Vector3 position_{ 0, 0, -5 }; // камера
		float yaw_ = 0.0f;   // вращение вокруг Y (влево/вправо)
		float pitch_ = 0.0f; // вращение вокруг X (вверх/вниз)

		Vector3 forward_{ 0,0,1 };
		Vector3 up_{ 0,1,0 };
		Vector3 right_{ 1,0,0 };
	};
}

