#pragma once
#include "Object3d.h"
#include "Sprite.h"
#include <memory>
#include "BaseCharacter.h"
#include "Bullet.h"
#include "LockOn.h"

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

	// 3Dレティクルの更新処理
	void UpdateReticle();

	// getter
	Object3d* GetObject3d() { return object3d.get(); }

private:
	// 3dオブジェクト
	std::unique_ptr<Object3d> object3d;
	// 当たり判定フラグ
	bool isHit_;
	// ファイル名
	std::string filePath_ = "cube.obj";
	std::string spriteFilePath_ = "resources/sprite/circle.png";
	// 弾の速度
	const float kBulletSpeed_ = 1.0f;
	// 移動限界
	const float kMoveLimitX_ = 7.0f;
	const float kMoveLimitY_ = 4.0f;
	// 3Dレティクルオブジェクト
	std::unique_ptr<Object3d> reticle_;
	// 3Dレティクルのワールドトランスフォーム
	QuaternionTransform reticleTransform_;
	// 自機から3Dレティクルまでの距離
	const float kDistance_ = 5.0f;
	// 自機から3Dレティクルへのオフセット
	Vector3 offset_ = { 0.0f,0.0f,5.0f };
	// ロックオンのポインタ
	std::unique_ptr<LockOn> lockOn_;

	Vector3 velocity;
	Vector3 targetPosition;
	bool isRailCamera;

};

