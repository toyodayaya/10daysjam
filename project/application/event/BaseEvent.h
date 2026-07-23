#pragma once
#include "BaseCharacter.h"

class BaseEvent : public BaseCharacter
{
public:
	// 初期化
	virtual void Initialize(const QuaternionTransform& transform) = 0;
	
private:
};

