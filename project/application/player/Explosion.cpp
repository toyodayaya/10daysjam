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
	effectElapsedFrames_ = 0;
	isEffectActive_ = false;
#endif
}

void Explosion::Update()
{
#ifdef _DEBUG
	if (debugSphere_ && isEffectActive_)
	{
		// 0～1の進行度を作り、Smoothstepで自然に収縮させる
		const float progress = static_cast<float>(effectElapsedFrames_) /
			static_cast<float>(kEffectDurationFrames_ - 1);
		const float smoothProgress = progress * progress * (3.0f - 2.0f * progress);
		const float scaleRate = 1.0f - smoothProgress;
		const float effectScale = radius_ * kInitialEffectScaleMultiplier_ * scaleRate;

		debugSphere_->SetScale({ effectScale, effectScale, effectScale });
		debugSphere_->Update();

		++effectElapsedFrames_;
		if (effectElapsedFrames_ >= kEffectDurationFrames_)
		{
			isEffectActive_ = false;
		}
	}
#endif
}

void Explosion::Draw()
{
#ifdef _DEBUG
	if (debugSphere_ && isEffectActive_)
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
		const float initialEffectScale = radius_ * kInitialEffectScaleMultiplier_;
		debugSphere_->SetScale({ initialEffectScale, initialEffectScale, initialEffectScale });
		effectElapsedFrames_ = 0;
		isEffectActive_ = true;
	}
#endif
}

void Explosion::Deactivate()
{
	isActive_ = false;
}

void Explosion::SetDamage(int damage)
{
	// ダメージが負数にならないように補正する
	damage_ = (std::max)(0, damage);
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
