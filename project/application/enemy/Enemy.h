#pragma once
#include "BaseEnemy.h"

class Enemy : public BaseEnemy
{
public:
	// 初期化
	void Initialize(const QuaternionTransform& transform, const std::string& filePath) override;
	// 終了
	void Finalize() override;
	// 更新
	void Update() override;
	// 描画
	void Draw() override;
	// 衝突応答
	void OnCollision(std::string hitObjectType) override;

	// getter
	Vector3 GetTranslate() { return transform_.translate; }


private:

};

