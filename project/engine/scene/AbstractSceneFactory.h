#pragma once
#include "DirectXBasis.h"
#include "BaseScene.h"
#include <string>

class AbstractSceneFactory
{
public:
	// 仮想デストラクタ
	virtual ~AbstractSceneFactory() = default;
	// シーンの生成
	virtual std::unique_ptr<BaseScene> CreateScene(const std::string& sceneName) = 0;
};