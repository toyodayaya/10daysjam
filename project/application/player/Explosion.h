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
	// 攻撃判定とは独立して、爆発範囲を約0.2秒表示する
	static constexpr int kDebugDisplayFrames_ = 12;
	int debugDisplayFrames_ = 0;
	std::unique_ptr<DebugDraw> debugSphere_;
#endif
};
