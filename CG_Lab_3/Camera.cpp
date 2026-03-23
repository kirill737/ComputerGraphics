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
		viewMatrix_ = Matrix::CreateLookAt(position_, target_, up_);
	}

	const Matrix& Camera::GetView() const
	{
		return Matrix::CreateLookAt(position_, position_ + forward_, up_);
	}

	const Matrix& Camera::GetProjection() const
	{
		return Matrix::CreatePerspectiveFieldOfView(
			DirectX::XM_PIDIV4, // 45 градусов
			screenPropotion_,
			0.1f, 100.0f);
	}

	void Camera::UpdateVectors()
	{
		DirectX::XMFLOAT3 worldUp = { 0.0f, 1.0f, 0.0f };
		forward_.x = cosf(pitch_) * sinf(yaw_);
		forward_.y = sinf(pitch_);
		forward_.z = cosf(pitch_) * cosf(yaw_);
		forward_.Normalize();

		right_ = forward_.Cross(up_);
		right_.Normalize();

		up_ = right_.Cross(forward_);
	}

	void Camera::Rotate(float deltaYaw, float deltaPitch)
	{
		yaw_ += deltaYaw;
		pitch_ += deltaPitch;

		// Ограничение угла по вертикали, чтобы не перевернуться
		if (pitch_ > DirectX::XM_PIDIV2 - 0.01f) pitch_ = DirectX::XM_PIDIV2 - 0.01f;
		if (pitch_ < -DirectX::XM_PIDIV2 + 0.01f) pitch_ = -DirectX::XM_PIDIV2 + 0.01f;

		UpdateVectors();
	}

}