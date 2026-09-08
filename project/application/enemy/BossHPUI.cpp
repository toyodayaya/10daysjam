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

	hpBarSprite_ = std::make_unique<Sprite>();
	hpBarSprite_->Initialize(SpriteCommon::GetInstance(), texturePath);
	hpBarSprite_->SetPosition(kBarPosition_);
	hpBarSprite_->SetSize(kBarSize_);
	// 左端を基準に幅を変え、HPが右側から減って見えるようにする
	hpBarSprite_->SetAnchorPoint({ 0.0f, 0.0f });
	hpBarSprite_->SetColor({ 0.9f, 0.12f, 0.08f, 1.0f });

	Update(0, 1);
}

void BossHPUI::Update(int currentHp, int maxHp)
{
	if (!frameSprite_ || !hpBarSprite_)
	{
		return;
	}

	const float hpRate = maxHp > 0
		? std::clamp(static_cast<float>(currentHp) / static_cast<float>(maxHp), 0.0f, 1.0f)
		: 0.0f;

	hpBarSprite_->SetSize({ kBarSize_.x * hpRate, kBarSize_.y });
	frameSprite_->Update();
	hpBarSprite_->Update();
}

void BossHPUI::Draw()
{
	if (!frameSprite_ || !hpBarSprite_)
	{
		return;
	}

	frameSprite_->Draw();
	hpBarSprite_->Draw();
}
