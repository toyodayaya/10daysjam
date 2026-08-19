#pragma once
#include "Object3d.h"
#include "BaseCharacter.h"

class BaseEnemy : public BaseCharacter
{
public:
	// 初期化
	virtual void Initialize(const QuaternionTransform& transform, const std::string& filePath) = 0;

	// getter
	Object3d* GetObject3d() { return object3d_.get(); }

protected:
	// モデルオブジェクト
	std::unique_ptr<Object3d> object3d_;
	// 当たり判定用のAABB
	AABB aabb_;
};

