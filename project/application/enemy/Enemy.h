#pragma once
#include "BaseEnemy.h"

class Enemy : public BaseEnemy
{
public:
	// 初期化
	void Initialize(const QuaternionTransform& transform, const std::string& filePath) override;
	// 終了
	void Finalize() override;
	// 更新
	void Update() override;
	// 描画
	void Draw() override;
	// 衝突応答
	void OnCollision(std::string hitObjectType, BaseCharacter* hitObject) override;

	// getter
	Vector3 GetTranslate() { return transform_.translate; }
	// HP加算関数
	void AddHP(const float& hp) override;
	// ダメージを受ける
	void TakeDamage(int damage) override;
	// 爆発との当たり判定に使用するAABBを取得
	AABB GetDamageAabb() const override;

private:
	// 初期HP
	const int kInitialHp_ = 30;
	// 現在HP
	int hp_ = kInitialHp_;
	// 爆発判定用AABBの中心から各面までの距離
	const Vector3 kDamageAabbHalfSize_ = { 1.0f, 1.0f, 1.0f };
};

