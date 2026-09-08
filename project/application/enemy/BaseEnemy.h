#pragma once
#include "Object3d.h"
#include "BaseCharacter.h"

class BaseEnemy : public BaseCharacter
{
public:
	// 初期化
	virtual void Initialize(const QuaternionTransform& transform, const std::string& filePath) = 0;
	// ダメージを受ける
	virtual void TakeDamage(int damage) = 0;
	// 自爆ダメージを受ける。ボス以外は通常ダメージと同じ扱いにする
	virtual void TakeExplosionDamage(int damage) { TakeDamage(damage); }
	// 爆発との当たり判定に使用するAABBを取得
	virtual AABB GetDamageAabb() const = 0;
	// ボス以外のEnemyは既定では画面UIを描画しない
	virtual void DrawUI() {}


protected:
	// 当たり判定用のAABB
	AABB aabb_;
};

