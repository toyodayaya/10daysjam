#pragma once
#include "Object3d.h"
#include <memory>
#include "BaseCharacter.h"
#include "Bullet.h"

class Player : public BaseCharacter
{
public:
	// 初期化
	void Initialize(const QuaternionTransform& transform, const std::string& filePath, bool isRailcamera);
	// 更新
	void Update() override;
	// 描画
	void Draw() override;
	// 終了
	void Finalize() override;

	// 衝突応答
	void OnCollision() override;

	// 弾の生成処理
	void CreateBullet();

	// getter
	Object3d* GetObject3d() { return object3d.get(); }

private:
	// 3dオブジェクト
	std::unique_ptr<Object3d> object3d;
	// 当たり判定フラグ
	bool isHit_;
	// ファイル名
	std::string filePath = "cube.obj";
	// 弾の速度
	const float kBulletSpeed_ = 1.0f;
	// 移動限界
	const float kMoveLimitX_ = 2.0f;
	const float kMoveLimitY_ = 2.0f;
};

