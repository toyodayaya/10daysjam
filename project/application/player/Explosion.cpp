#include "Explosion.h"

#include <algorithm>

#ifdef _DEBUG
#include "DebugDraw.h"
#include "DebugDrawCommon.h"
#endif

Explosion::Explosion(float radius, int damage)
	: radius_(radius), damage_(damage)
{}

Explosion::~Explosion() = default;

void Explosion::Initialize()
{
	isActive_ = false;

#ifdef _DEBUG
	debugSphere_ = std::make_unique<DebugDraw>();
	debugSphere_->Initialize(
		DebugDrawCommon::GetInstance(),
		"resources/human/white.png",
		DebugDraw::DrawState::kSphere);
	debugSphere_->SetScale({ radius_, radius_, radius_ });
	debugSphere_->SetRotate({ 0.0f, 0.0f, 0.0f, 1.0f });
	debugSphere_->SetTranslate(center_);
	debugSphere_->SetParent(nullptr);
	debugSphere_->SetIsRailCamera(false);
	debugDisplayFrames_ = 0;
#endif
}

void Explosion::Update()
{
#ifdef _DEBUG
	if (debugSphere_ && debugDisplayFrames_ > 0)
	{
		debugSphere_->Update();

		// 発生フレームを含めて約0.2秒表示する
		if (!isActive_)
		{
			--debugDisplayFrames_;
		}
	}
#endif
}

void Explosion::Draw()
{
#ifdef _DEBUG
	if (debugSphere_ && debugDisplayFrames_ > 0)
	{
		debugSphere_->Draw();
	}
#endif
}

void Explosion::Activate(const Vector3& center)
{
	center_ = center;
	isActive_ = true;

#ifdef _DEBUG
	if (debugSphere_)
	{
		debugSphere_->SetTranslate(center_);
		debugSphere_->SetScale({ radius_, radius_, radius_ });
		debugDisplayFrames_ = kDebugDisplayFrames_;
	}
#endif
}

void Explosion::Deactivate()
{
	isActive_ = false;
}

bool Explosion::IsCollision(const AABB& aabb) const
{
	if (!isActive_)
	{
		return false;
	}

	// 球の中心に最も近いAABB上の点を求める
	const Vector3 closestPoint = {
		std::clamp(center_.x, aabb.min.x, aabb.max.x),
		std::clamp(center_.y, aabb.min.y, aabb.max.y),
		std::clamp(center_.z, aabb.min.z, aabb.max.z)
	};
	const Vector3 difference = MathManager::Vector3Subtract(center_, closestPoint);

	return MathManager::LengthSquared(difference) <= radius_ * radius_;
}

bool Explosion::IsActive() const
{
	return isActive_;
}

const Vector3& Explosion::GetCenter() const
{
	return center_;
}

float Explosion::GetRadius() const
{
	return radius_;
}

int Explosion::GetDamage() const
{
	return damage_;
}
