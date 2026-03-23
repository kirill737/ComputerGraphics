#pragma once

#include "SphereComponent.h"

namespace CGLib {
	class OrbitingSphere : public SphereComponent {
	public:
		OrbitingSphere(std::shared_ptr<GameComponent> orbitCenter,
			float orbitRadius,
			float orbitSpeed,   // радианы/сек
			float sphereRadius)
			: orbitCenter_(orbitCenter)
			, orbitRadius_(orbitRadius)
			, orbitSpeed_(orbitSpeed)
		{
			radius_ = sphereRadius;
		}

		void Update(float deltaTime) override
		{
			if (orbitCenter_) {
				angle_ += orbitSpeed_ * deltaTime;

				// создаём кватернион вращения вокруг заданной оси
				DirectX::SimpleMath::Quaternion q =
					DirectX::SimpleMath::Quaternion::CreateFromAxisAngle(orbitAxis_, angle_);

				// смещённый вектор от центра орбиты
				DirectX::SimpleMath::Vector3 offset =
					DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3(orbitRadius_, 0, 0), q);

				SetPos(orbitCenter_->GetPos() + offset);
			}

			SphereComponent::Update(deltaTime);
		}

		void SetOrbitAxis(const DirectX::SimpleMath::Vector3& axis) {
			orbitAxis_ = axis;
			orbitAxis_.Normalize();
		}

	private:
		std::shared_ptr<GameComponent> orbitCenter_ = nullptr;
		float orbitRadius_;
		float orbitSpeed_;
		DirectX::SimpleMath::Vector3 orbitAxis_ = DirectX::SimpleMath::Vector3::Up;
		float angle_ = 0.0f;
	};
}