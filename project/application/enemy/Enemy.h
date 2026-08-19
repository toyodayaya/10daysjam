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
	void OnCollision() override;

	// getter
	Vector3 GetTranslate() { return translate; }


private:
	// 座標
	Vector3 translate;
};

