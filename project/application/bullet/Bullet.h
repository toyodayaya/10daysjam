#pragma once
#include "BaseBullet.h"

class Bullet : public BaseBullet
{
public:
	// 初期化
	void Initialize(const QuaternionTransform& transform, const std::string& filePath, const Vector3& velocity) override;
	// 終了
	void Finalize() override;
	// 更新
	void Update() override;
	// 描画
	void Draw() override;
	// 衝突応答
	void OnCollision() override;

private:
	
};

