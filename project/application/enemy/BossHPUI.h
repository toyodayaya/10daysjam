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
	// 自爆命中時のシェイクを開始する
	void StartShake();
	// 2Dスプライトを描画する
	void Draw();

private:
	// 1280x720の画面中央を基準に配置する
	static constexpr Vector2 kFramePosition_ = { 336.0f, 32.0f };
	static constexpr Vector2 kFrameSize_ = { 608.0f, 32.0f };
	static constexpr Vector2 kBarPosition_ = { 340.0f, 36.0f };
	static constexpr Vector2 kBarSize_ = { 600.0f, 24.0f };
	// 被弾直後にダメージ部分をそのまま見せる時間（60FPSで約0.2秒）
	static constexpr int kDamageHoldFrames_ = 12;
	// 濃い赤のバーが実HPまで追従する時間（60FPSで約0.5秒）
	static constexpr int kDamageAnimationFrames_ = 30;
	// 自爆命中時のシェイク時間（60FPSで約0.2秒）
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
	int shakeElapsedFrames_ = 0;
	bool isShakeActive_ = false;
};
