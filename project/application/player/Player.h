#pragma once
#include "Object3d.h"
#include <memory>
#include "BaseCharacter.h"

class Player : public BaseCharacter
{
public:
	// 初期化
	void Initialize(const QuaternionTransform& transform, const std::string& filePath);
	// 更新
	void Update() override;
	// 描画
	void Draw() override;
	// 終了
	void Finalize() override;

	// 衝突応答
	void OnCollision() override;

	// getter
	Object3d* GetObject3d() { return object3d.get(); }

private:
	// 3dオブジェクト
	std::unique_ptr<Object3d> object3d;
	// 当たり判定フラグ
	bool isHit_;
};

