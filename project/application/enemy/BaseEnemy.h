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
	// 爆発との当たり判定に使用するAABBを取得
	virtual AABB GetDamageAabb() const = 0;


protected:
	// 当たり判定用のAABB
	AABB aabb_;
};

