#pragma once
#include "Object3d.h"
#include "BaseCharacter.h"

class BaseEnemy : public BaseCharacter
{
public:
	// 初期化
	virtual void Initialize(const QuaternionTransform& transform, const std::string& filePath) = 0;


protected:
	// 当たり判定用のAABB
	AABB aabb_;
};

