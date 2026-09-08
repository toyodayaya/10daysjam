#include "PlayerHPUI.h"

#include "Sprite.h"
#include "SpriteCommon.h"
#include "TextureManager.h"

#include <algorithm>

void PlayerHPUI::Initialize()
{
	// 白画像を色付けして、背景とHP残量の2枚のスプライトとして使用する
	const std::string texturePath = "resources/human/white.png";
	TextureManager::GetInstance()->LoadTexture(texturePath);

	frameSprite_ = std::make_unique<Sprite>();
	frameSprite_->Initialize(SpriteCommon::GetInstance(), texturePath);
	frameSprite_->SetPosition(kFramePosition_);
	frameSprite_->SetSize(kFrameSize_);
	frameSprite_->SetAnchorPoint({ 0.0f, 0.0f });
	frameSprite_->SetColor({ 0.08f, 0.08f, 0.08f, 0.85f });

	hpBarSprite_ = std::make_unique<Sprite>();
	hpBarSprite_->Initialize(SpriteCommon::GetInstance(), texturePath);
	hpBarSprite_->SetPosition(kBarPosition_);
	hpBarSprite_->SetSize(kBarSize_);
	// 左端を基準に幅を変えることで、HPが右から減って見えるようにする
	hpBarSprite_->SetAnchorPoint({ 0.0f, 0.0f });

	Update(0, 1);
}

void PlayerHPUI::Update(int currentHp, int maxHp)
{
	if (!frameSprite_ || !hpBarSprite_)
	{
		return;
	}

	// 最大HPが0以下でもゼロ除算しないようにし、表示割合は必ず0～1に収める
	const float hpRate = maxHp > 0
		? std::clamp(static_cast<float>(currentHp) / static_cast<float>(maxHp), 0.0f, 1.0f)
		: 0.0f;

	hpBarSprite_->SetSize({ kBarSize_.x * hpRate, kBarSize_.y });

	// HPが少ないと赤、HPが多いと緑になるように色を補間する
	const Vector4 lowHpColor = { 0.9f, 0.12f, 0.08f, 1.0f };
	const Vector4 highHpColor = { 0.12f, 0.85f, 0.24f, 1.0f };
	hpBarSprite_->SetColor({
		lowHpColor.x + (highHpColor.x - lowHpColor.x) * hpRate,
		lowHpColor.y + (highHpColor.y - lowHpColor.y) * hpRate,
		lowHpColor.z + (highHpColor.z - lowHpColor.z) * hpRate,
		1.0f
	});

	frameSprite_->Update();
	hpBarSprite_->Update();
}

void PlayerHPUI::Draw()
{
	if (!frameSprite_ || !hpBarSprite_)
	{
		return;
	}

	// 背景を先に描き、その上へ現在HPのバーを重ねる
	frameSprite_->Draw();
	hpBarSprite_->Draw();
}
