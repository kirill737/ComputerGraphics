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

		// движение только по плоскости XZ
		move.y = 0.0f;

		if (move.LengthSquared() > 0.0f)
		{
			move.Normalize();

			Vector3 delta = move * moveSpeed_ * deltaTime;
			pos_ += delta;

			// шар всегда касается земли
			pos_.y = radius_;

			// вращение от качения, а не просто так
			float distance = delta.Length();
			float rotationAngle = distance / radius_;

			// ось вращения перпендикулярна направлению движения
			Vector3 rotationAxis = Vector3::Up.Cross(move);
			if (rotationAxis.LengthSquared() > 0.0f)
			{
				rotationAxis.Normalize();
				SetSelfRotationAxis(rotationAxis);
				selfRotationAngle_ += rotationAngle;
			}

			UpdateWorldMatrix();
		}
		else
		{
			// даже если не двигается, держим шар на земле
			pos_.y = radius_;
			UpdateWorldMatrix();
		}
	}
}