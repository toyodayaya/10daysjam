#pragma once
#include "BaseCharacter.h"

class BaseEvent : public BaseCharacter
{
public:
	// 初期化
	virtual void Initialize(const QuaternionTransform& transform, const std::string& filePath) = 0;
	
private:
};

