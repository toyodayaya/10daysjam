#pragma once
#include "BaseEnemy.h"

class Enemy : public BaseEnemy
{
public:
	// 初期化
	void Initialize(QuaternionTransform transform, const std::string& filePath) override;
	// 終了
	void Finalize() override;
	// 更新
	void Update() override;
	// 描画
	void Draw() override;

private:
	
};

