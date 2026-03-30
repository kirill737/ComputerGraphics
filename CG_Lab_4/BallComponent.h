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

	private:
		DirectX::SimpleMath::Vector3 moveInput_{ 0.0f, 0.0f, 0.0f };
		float moveSpeed_ = 10000.0f;
	};
}