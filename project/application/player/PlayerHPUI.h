#pragma once

#include "MathManager.h"

#include <memory>

class Sprite;

// Playerの現在HPを画面右下にバーとして表示するUI
class PlayerHPUI
{
public:
	// HPバーで使用するSpriteを初期化する
	void Initialize();
	// PlayerのHPを受け取り、バーの長さと色を更新する
	void Update(int currentHp, int maxHp);
	// 2Dスプライトを描画する
	void Draw();

private:
	// 枠を少し大きくすることで、HPバーの背景兼ボーダーとして表示する
	static constexpr Vector2 kFramePosition_ = { 940.0f, 656.0f };
	static constexpr Vector2 kFrameSize_ = { 308.0f, 32.0f };
	static constexpr Vector2 kBarPosition_ = { 944.0f, 660.0f };
	static constexpr Vector2 kBarSize_ = { 300.0f, 24.0f };

	std::unique_ptr<Sprite> frameSprite_;
	std::unique_ptr<Sprite> hpBarSprite_;
};
