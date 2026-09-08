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
	// 敵の攻撃を受けた時のダメージ残像とシェイクを開始する
	void StartDamageEffect();
	// 2Dスプライトを描画する
	void Draw();

private:
	// 枠を少し大きくすることで、HPバーの背景兼ボーダーとして表示する
	static constexpr Vector2 kFramePosition_ = { 940.0f, 656.0f };
	static constexpr Vector2 kFrameSize_ = { 308.0f, 32.0f };
	static constexpr Vector2 kBarPosition_ = { 944.0f, 660.0f };
	static constexpr Vector2 kBarSize_ = { 300.0f, 24.0f };
	// 被弾直後にダメージ部分をそのまま見せる時間（60FPSで約0.2秒）
	static constexpr int kDamageHoldFrames_ = 12;
	// 濃い赤のバーが実HPまで追従する時間（60FPSで約0.5秒）
	static constexpr int kDamageAnimationFrames_ = 30;
	// 被弾時のシェイク時間（60FPSで約0.2秒）
	static constexpr int kShakeDurationFrames_ = 12;
	// UIが大きくずれすぎないようにする最大振幅
	static constexpr float kShakeAmplitude_ = 5.0f;

	std::unique_ptr<Sprite> frameSprite_;
	std::unique_ptr<Sprite> damageBarSprite_;
	std::unique_ptr<Sprite> hpBarSprite_;

	float currentHpRate_ = 0.0f;
	float damageBarRate_ = 0.0f;
	float damageAnimationStartRate_ = 0.0f;
	int damageHoldFrames_ = 0;
	int damageAnimationElapsedFrames_ = 0;
	bool isDamageAnimationActive_ = false;
	bool isDamageEffectPending_ = false;
	int shakeElapsedFrames_ = 0;
	bool isShakeActive_ = false;
};
