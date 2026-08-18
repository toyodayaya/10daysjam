#pragma once
#include "Object3d.h"
#include "BaseCharacter.h"
#ifdef _DEBUG
#include "DebugDraw.h"
#include "DebugDrawCommon.h"
#endif // _DEBUG

class BaseBullet : public BaseCharacter
{
public:
	// 初期化
	virtual void Initialize(const QuaternionTransform& transform, const std::string& filePath,const Vector3& velocity) = 0;
	// getter
	bool IsDead() { return isDead_; }

protected:
	// モデルオブジェクト
	std::unique_ptr<Object3d> object3d_;
	// 当たり判定用のAABB
	AABB aabb_;
	// 移動速度
	Vector3 velocity_;
	// 生存時間
	float currentTime_ = 0;
	const float kLifeTime_ = 5;
	// デスフラグ
	bool isDead_ = false;
#ifdef _DEBUG
	std::unique_ptr<DebugDraw> debugDraw;
#endif // _DEBUG

};

