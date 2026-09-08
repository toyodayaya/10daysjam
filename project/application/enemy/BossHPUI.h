#pragma once

#include "MathManager.h"

#include <memory>

class Sprite;

// ボスの現在HPを画面中央上にバーとして表示するUI
class BossHPUI
{
public:
	// HPバーで使用するSpriteを初期化する
	void Initialize();
	// ボスのHPを受け取り、バーの長さを更新する
	void Update(int currentHp, int maxHp);
	// 2Dスプライトを描画する
	void Draw();

private:
	// 1280x720の画面中央を基準に配置する
	static constexpr Vector2 kFramePosition_ = { 336.0f, 32.0f };
	static constexpr Vector2 kFrameSize_ = { 608.0f, 32.0f };
	static constexpr Vector2 kBarPosition_ = { 340.0f, 36.0f };
	static constexpr Vector2 kBarSize_ = { 600.0f, 24.0f };

	std::unique_ptr<Sprite> frameSprite_;
	std::unique_ptr<Sprite> hpBarSprite_;
};
