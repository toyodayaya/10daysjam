#include "BossHPUI.h"

#include "Sprite.h"
#include "SpriteCommon.h"
#include "TextureManager.h"

#include <algorithm>

void BossHPUI::Initialize()
{
	// 白画像を色付けして、背景とHP残量の2枚のスプライトとして使用する
	const std::string texturePath = "resources/human/white.png";
	TextureManager::GetInstance()->LoadTexture(texturePath);

	frameSprite_ = std::make_unique<Sprite>();
	frameSprite_->Initialize(SpriteCommon::GetInstance(), texturePath);
	frameSprite_->SetPosition(kFramePosition_);
	frameSprite_->SetSize(kFrameSize_);
	frameSprite_->SetAnchorPoint({ 0.0f, 0.0f });
	frameSprite_->SetColor({ 0.08f, 0.08f, 0.08f, 0.9f });

	// 実HPバーの背面に置き、被弾で失った部分だけを濃い赤として見せる
	damageBarSprite_ = std::make_unique<Sprite>();
	damageBarSprite_->Initialize(SpriteCommon::GetInstance(), texturePath);
	damageBarSprite_->SetPosition(kBarPosition_);
	damageBarSprite_->SetSize(kBarSize_);
	damageBarSprite_->SetAnchorPoint({ 0.0f, 0.0f });
	damageBarSprite_->SetColor({ 0.45f, 0.02f, 0.02f, 1.0f });

	hpBarSprite_ = std::make_unique<Sprite>();
	hpBarSprite_->Initialize(SpriteCommon::GetInstance(), texturePath);
	hpBarSprite_->SetPosition(kBarPosition_);
	hpBarSprite_->SetSize(kBarSize_);
	// 左端を基準に幅を変え、HPが右側から減って見えるようにする
	hpBarSprite_->SetAnchorPoint({ 0.0f, 0.0f });
	hpBarSprite_->SetColor({ 0.9f, 0.12f, 0.08f, 1.0f });

	currentHpRate_ = 0.0f;
	damageBarRate_ = 0.0f;
	damageAnimationStartRate_ = 0.0f;
	damageHoldFrames_ = 0;
	damageAnimationElapsedFrames_ = 0;
	isDamageAnimationActive_ = false;
	Update(0, 1);
}

void BossHPUI::Update(int currentHp, int maxHp)
{
	if (!frameSprite_ || !damageBarSprite_ || !hpBarSprite_)
	{
		return;
	}

	const float hpRate = maxHp > 0
		? std::clamp(static_cast<float>(currentHp) / static_cast<float>(maxHp), 0.0f, 1.0f)
		: 0.0f;

	const bool tookDamage = hpRate < currentHpRate_;
	if (tookDamage)
	{
		// 実HPは即座に減らし、濃い赤のバーは被弾前の位置に残す
		currentHpRate_ = hpRate;
		damageAnimationStartRate_ = damageBarRate_;
		damageHoldFrames_ = kDamageHoldFrames_;
		damageAnimationElapsedFrames_ = 0;
		isDamageAnimationActive_ = damageBarRate_ > currentHpRate_;
	}
	else if (hpRate > currentHpRate_)
	{
		// 回復した場合は2本のバーを新しいHPへ即座に揃える
		currentHpRate_ = hpRate;
		damageBarRate_ = hpRate;
		damageAnimationStartRate_ = hpRate;
		damageHoldFrames_ = 0;
		damageAnimationElapsedFrames_ = 0;
		isDamageAnimationActive_ = false;
	}

	if (isDamageAnimationActive_ && !tookDamage)
	{
		if (damageHoldFrames_ > 0)
		{
			--damageHoldFrames_;
		}
		else
		{
			++damageAnimationElapsedFrames_;
			const float progress = std::clamp(
				static_cast<float>(damageAnimationElapsedFrames_) /
				static_cast<float>(kDamageAnimationFrames_),
				0.0f, 1.0f);
			const float smoothProgress = progress * progress * (3.0f - 2.0f * progress);
			damageBarRate_ = damageAnimationStartRate_ +
				(currentHpRate_ - damageAnimationStartRate_) * smoothProgress;

			if (progress >= 1.0f)
			{
				damageBarRate_ = currentHpRate_;
				isDamageAnimationActive_ = false;
			}
		}
	}

	hpBarSprite_->SetSize({ kBarSize_.x * currentHpRate_, kBarSize_.y });
	damageBarSprite_->SetSize({ kBarSize_.x * damageBarRate_, kBarSize_.y });
	frameSprite_->Update();
	damageBarSprite_->Update();
	hpBarSprite_->Update();
}

void BossHPUI::Draw()
{
	if (!frameSprite_ || !damageBarSprite_ || !hpBarSprite_)
	{
		return;
	}

	frameSprite_->Draw();
	damageBarSprite_->Draw();
	hpBarSprite_->Draw();
}
