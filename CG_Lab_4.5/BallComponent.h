#pragma once

#include "SphereComponent.h"

namespace CGLib
{
	class BallComponent : public SphereComponent
	{
	public:
		BallComponent(float radius, int sliceCount = 16, int stackCount = 16)
			: SphereComponent(radius, sliceCount, stackCount)
		{
			SetSelfRotationEnabled(false);
		}

		void Update(float deltaTime) override;

		void SetMoveInput(const DirectX::SimpleMath::Vector3& inputDir);
		void SetMoveSpeed(float speed) { moveSpeed_ = speed; }
		float GetMoveSpeed() const { return moveSpeed_; }

		void RequestJump()
		{
			jumpRequested_ = true;
		}

		bool IsGrounded() const
		{
			return pos_.y <= radius_ + 0.001f;
		}

	private:
		DirectX::SimpleMath::Vector3 moveInput_{ 0.0f, 0.0f, 0.0f };
		float moveSpeed_ = 10.0f;

		float verticalVelocity_ = 0.0f;
		float gravity_ = -20.0f;
		float jumpSpeed_ = 8.0f;

		int jumpCount_ = 0;
		int maxJumps_ = 5;

		bool jumpRequested_ = false;
	};
}