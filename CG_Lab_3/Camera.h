#pragma once

#include "SimpleMath.h"
#include "InputDevice.h"
#include <iostream>

namespace CGLib
{
	using namespace DirectX::SimpleMath;

	enum class CameraMode
	{
		Free,
		Orbit
	};

	enum class ProjectionMode
	{
		Perspective,
		Orthographic
	};

	class Camera final
	{
	public:
		Camera();
		Camera(const float& screenWidth, const float& screenHeight);

		void SetPos(const Vector3& pos);
		void LookAt(const Vector3& target);

		const Matrix& GetView() const;
		const Matrix& GetProjection() const;

		void UpdateViewMatrix();
		void UpdateProjection();

		void MoveForward(float dt, float speed) { position_ += forward_ * speed * dt; UpdateViewMatrix(); }
		void MoveBackward(float dt, float speed) { position_ -= forward_ * speed * dt; UpdateViewMatrix(); }
		void MoveRight(float dt, float speed) { position_ += right_ * speed * dt; UpdateViewMatrix(); }
		void MoveLeft(float dt, float speed) { position_ -= right_ * speed * dt; UpdateViewMatrix(); }
		void MoveUp(float dt, float speed) { position_ += up_ * speed * dt; UpdateViewMatrix(); }
		void MoveDown(float dt, float speed) { position_ -= up_ * speed * dt; UpdateViewMatrix(); }

		void UpdateVectors();
		void Rotate(float deltaYaw, float deltaPitch);

		void ToggleMode();
		void ToggleProjection();

		CameraMode GetMode() const { return mode_; }

		void Zoom(float delta);

		void UpdateOrbit();

	private:
		float screenPropotion_ = 1.0f;

		Vector3 target_{ 0.0f, 0.0f, 0.0f };

		Matrix viewMatrix_;
		Matrix projectionMatrix_;
		InputDevice* input_ = nullptr;

		float fov_ = DirectX::XM_PIDIV4;
		Vector3 position_{ 0, 0, -5 };
		float yaw_ = 0.0f;
		float pitch_ = 0.0f;

		Vector3 forward_{ 0,0,1 };
		Vector3 up_{ 0,1,0 };
		Vector3 right_{ 1,0,0 };

		ProjectionMode projectionMode_ = ProjectionMode::Perspective;
		CameraMode mode_ = CameraMode::Free;

		float orbitRadius_ = 10.0f;
	};
}