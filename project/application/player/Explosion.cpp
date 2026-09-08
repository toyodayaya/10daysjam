#include "Explosion.h"

#include <algorithm>
#include "Model.h"
#include "ModelManager.h"
#include "Object3d.h"
#include "Object3dCommon.h"

Explosion::Explosion(float radius, int damage)
	: radius_(radius), damage_(damage)
{}

Explosion::~Explosion() = default;

void Explosion::Initialize()
{
	isActive_ = false;

	ModelManager::GetInstance()->LoadModel(
		"resources/explosion", "explosion.obj", Model::AnimationType::kNone);

	effectObject_ = std::make_unique<Object3d>();
	effectObject_->Initialize(Object3dCommon::GetInstance());
	effectObject_->SetModel("explosion.obj");
	effectObject_->SetEnvironmentMapTextureFilePath("resources/human/white.png");
	effectObject_->SetScale({ radius_, radius_, radius_ });
	effectObject_->SetRotate({ 0.0f, 0.0f, 0.0f, 1.0f });
	effectObject_->SetTranslate(center_);
	effectObject_->SetParent(nullptr);
	effectObject_->SetIsRailCamera(false);
	effectElapsedFrames_ = 0;
	isEffectActive_ = false;
}

void Explosion::Update()
{
	if (effectObject_ && isEffectActive_)
	{
		// 0～1の進行度を作り、Smoothstepで自然に収縮させる
		const float progress = static_cast<float>(effectElapsedFrames_) /
			static_cast<float>(kEffectDurationFrames_ - 1);
		const float smoothProgress = progress * progress * (3.0f - 2.0f * progress);
		const float scaleRate = 1.0f - smoothProgress;
		const float effectScale = radius_ * kInitialEffectScaleMultiplier_ * scaleRate;

		effectObject_->SetScale({ effectScale, effectScale, effectScale });
		effectObject_->Update();

		++effectElapsedFrames_;
		if (effectElapsedFrames_ >= kEffectDurationFrames_)
		{
			isEffectActive_ = false;
		}
	}
}

void Explosion::Draw()
{
	if (effectObject_ && isEffectActive_)
	{
		effectObject_->Draw();
	}
}

void Explosion::Activate(const Vector3& center)
{
	center_ = center;
	isActive_ = true;
	if (effectObject_)
	{
		effectObject_->SetTranslate(center_);
		const float initialEffectScale = radius_ * kInitialEffectScaleMultiplier_;
		effectObject_->SetScale({ initialEffectScale, initialEffectScale, initialEffectScale });
		effectElapsedFrames_ = 0;
		isEffectActive_ = true;
	}
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
