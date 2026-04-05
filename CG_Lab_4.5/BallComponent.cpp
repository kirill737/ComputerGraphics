#include "BallComponent.h"
#include <DirectXMath.h>

namespace CGLib
{
	using namespace DirectX::SimpleMath;

	void BallComponent::SetMoveInput(const Vector3& inputDir)
	{
		moveInput_ = inputDir;
	}

	void BallComponent::Update(float deltaTime)
	{
		Vector3 move = moveInput_;
		move.y = 0.0f;

		if (move.LengthSquared() > 0.0f)
		{
			move.Normalize();

			Vector3 delta = move * moveSpeed_ * deltaTime;
			pos_ += delta;

			float distance = delta.Length();
			float rotationAngle = distance / radius_;

			Vector3 rotationAxis = Vector3::Up.Cross(move);
			if (rotationAxis.LengthSquared() > 0.0f)
			{
				rotationAxis.Normalize();

				Quaternion deltaRotation =
					Quaternion::CreateFromAxisAngle(rotationAxis, rotationAngle);

				rotation_ = rotation_ * deltaRotation;
				rotation_.Normalize();
			}
		}

		// Прыжок
		if (jumpRequested_)
		{
			if (jumpCount_ < maxJumps_)
			{
				verticalVelocity_ = jumpSpeed_;
				jumpCount_++;
			}
			jumpRequested_ = false;
		}

		// Гравитация
		verticalVelocity_ += gravity_ * deltaTime;
		pos_.y += verticalVelocity_ * deltaTime;

		// Столкновение с землёй
		if (pos_.y <= radius_)
		{
			pos_.y = radius_;
			verticalVelocity_ = 0.0f;
			jumpCount_ = 0;
		}

		UpdateWorldMatrix();
	}
}