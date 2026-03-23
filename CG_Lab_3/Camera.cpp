#include "Camera.h"

namespace CGLib
{

	Camera::Camera()
	{
		position_ = Vector3(0.0f, 0.0f, -10.0f);
		target_ = Vector3::Zero;
		up_ = Vector3::UnitY;

		viewMatrix_ = Matrix::CreateLookAt(position_, target_, up_);

		projectionMatrix_ =
			Matrix::CreatePerspectiveFieldOfView(
				DirectX::XM_PIDIV4,
				16.0f / 9.0f,
				0.1f,
				100.0f
			);
	}

	void Camera::SetPos(const Vector3& pos)
	{
		position_ = pos;
		UpdateViewMatrix();
	}

	void Camera::LookAt(const Vector3& target)
	{
		target_ = target;
		UpdateViewMatrix();
	}

	void Camera::UpdateViewMatrix()
	{
		if (mode_ == CameraMode::Orbit)
		{
			// орбитальная камера — смотри на target_
			viewMatrix_ = Matrix::CreateLookAt(position_, target_, Vector3::Up);
		}
		else
		{
			// free-камера — классический FPS-стиль
			viewMatrix_ = Matrix::CreateLookAt(position_, position_ + forward_, up_);
		}
	}

	const Matrix& Camera::GetView() const
	{
		return viewMatrix_;
	}

	const Matrix& Camera::GetProjection() const
	{
		return Matrix::CreatePerspectiveFieldOfView(
			DirectX::XM_PIDIV4, // 45 градусов
			screenPropotion_,
			0.1f, 100.0f);
	}
	/*const Matrix& Camera::GetProjection() const
	{
		return projectionMatrix_;
	}*/

	void Camera::UpdateVectors()
	{
		Vector3 worldUp = Vector3::Up; // фиксированный up

		// пересчёт направления взгляда
		forward_.x = cosf(pitch_) * sinf(yaw_);
		forward_.y = sinf(pitch_);
		forward_.z = cosf(pitch_) * cosf(yaw_);
		forward_.Normalize();

		// right — всегда перпендикулярен worldUp и forward
		right_ = forward_.Cross(worldUp);
		right_.Normalize();

		// up — корректный
		up_ = right_.Cross(forward_);
		up_.Normalize();
	}

	void Camera::Rotate(float deltaYaw, float deltaPitch)
	{
		yaw_ += deltaYaw;
		pitch_ += deltaPitch;

		if (pitch_ > DirectX::XM_PIDIV2 - 0.01f) pitch_ = DirectX::XM_PIDIV2 - 0.01f;
		if (pitch_ < -DirectX::XM_PIDIV2 + 0.01f) pitch_ = -DirectX::XM_PIDIV2 + 0.01f;

		if (mode_ == CameraMode::Orbit)
			UpdateOrbit();
		else
		{
			UpdateVectors();      // пересчитываем forward/right/up
			UpdateViewMatrix();   // используем forward вместо target
		}
	}

	void Camera::ToggleMode()
	{
		if (mode_ == CameraMode::Free)
		{
			// переключаемся на орбитальную
			mode_ = CameraMode::Orbit;
			std::cout << "Orbital cam\n";

			orbitRadius_ = (position_ - target_).Length();
			UpdateOrbit();
		}
		else
		{
			mode_ = CameraMode::Free;
			std::cout << "Free cam\n";

			// Сохраняем текущую орбитальную дистанцию, чтобы камера была снаружи
			if (orbitRadius_ < 2.0f) orbitRadius_ = 2.0f;

			// Перемещаем камеру назад от target на orbitRadius_
			position_ = target_ - forward_ * orbitRadius_;

			// Пересчёт forward/right/up для free-режима
			Vector3 lookDir = (target_ - position_);
			lookDir.Normalize();
			forward_ = lookDir;
			right_ = forward_.Cross(Vector3::Up);
			right_.Normalize();
			up_ = right_.Cross(forward_);
			up_.Normalize();

			UpdateViewMatrix();
		}
	}

	void Camera::ToggleProjection()
	{
		if (projectionMode_ == ProjectionMode::Perspective) {
			std::cout << "Orto mode\n";

			projectionMode_ = ProjectionMode::Orthographic;
		}
		
		else {
			std::cout << "Perspective mode\n";
			projectionMode_ = ProjectionMode::Perspective;
		}
			

		UpdateProjection();
	}

	void Camera::UpdateProjection()
	{
		if (projectionMode_ == ProjectionMode::Perspective)
		{
			projectionMatrix_ = Matrix::CreatePerspectiveFieldOfView(
				fov_,
				screenPropotion_,
				0.1f,
				100.0f
			);
		}
		else
		{
			float orthoWidth = 10.0f;   // ширина/высота видимой области
			float orthoHeight = orthoWidth / screenPropotion_;
			projectionMatrix_ = Matrix::CreateOrthographic(
				orthoWidth,
				orthoHeight,
				0.1f,
				100.0f
			);
		}
	}


	void Camera::UpdateOrbit()
	{
		Vector3 offset;

		offset.x = orbitRadius_ * cosf(pitch_) * sinf(yaw_);
		offset.y = orbitRadius_ * sinf(pitch_);
		offset.z = orbitRadius_ * cosf(pitch_) * cosf(yaw_);

		position_ = target_ + offset;

		viewMatrix_ = Matrix::CreateLookAt(position_, target_, Vector3::Up);
	}

}