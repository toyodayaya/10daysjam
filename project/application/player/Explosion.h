#pragma once

#include "MathManager.h"
#include <memory>

class DebugDraw;

// 球形爆発攻撃
class Explosion
{
public:
	Explosion(float radius, int damage);
	~Explosion();

	// 初期化
	void Initialize();
	// 更新
	void Update();
	// 描画
	void Draw();

	// 爆発を発生させる
	void Activate(const Vector3& center);

	// 爆発判定を終了する
	void Deactivate();
	// 爆発のダメージを設定する
	void SetDamage(int damage);

	// 球とAABBの当たり判定
	bool IsCollision(const AABB& aabb) const;

	bool IsActive() const;
	const Vector3& GetCenter() const;
	float GetRadius() const;
	int GetDamage() const;

private:
	Vector3 center_ = { 0.0f, 0.0f, 0.0f };
	float radius_ = 0.0f;
	int damage_ = 0;
	bool isActive_ = false;

#ifdef _DEBUG
	// 攻撃判定とは独立して、爆発の収縮演出を約0.3秒再生する
	static constexpr int kEffectDurationFrames_ = 18;
	// 爆発した瞬間の見た目を攻撃範囲より少し大きくする
	static constexpr float kInitialEffectScaleMultiplier_ = 1.25f;
	int effectElapsedFrames_ = 0;
	bool isEffectActive_ = false;
	std::unique_ptr<DebugDraw> debugSphere_;
#endif
};
