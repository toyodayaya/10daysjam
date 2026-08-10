#pragma once
#include "Object3d.h"
#include "BaseCharacter.h"

class BaseBullet : public BaseCharacter
{
public:
	// 初期化
	virtual void Initialize(const QuaternionTransform& transform, const std::string& filePath) = 0;

protected:
	// モデルオブジェクト
	std::unique_ptr<Object3d> object3d_;
	// 当たり判定用のAABB
	AABB aabb_;
};

