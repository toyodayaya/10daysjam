#pragma once
#include "Object3d.h"
#include "Sprite.h"
#include <memory>
#include"BaseEnemy.h"

class LockOn
{
public:
	// 初期化
	void Initialize();
	// 更新
	void Update();
	// 描画
	void Draw();
	// 終了
	void Finalize();

	// ターゲット算出
	void LockOnTarget(const std::unique_ptr<Object3d>& reticle);

	// getter
	BaseEnemy* GetTarget() const { return target_; }

private:
	// 当たり判定フラグ
	bool isHit_;
	// ファイル名
	std::string spriteFilePath_ = "resources/sprite/circle.png";
	// 弾の速度
	// 3Dレティクル用のスプライト
	std::unique_ptr<Sprite> reticleSprite_;
	// スプライトの座標
	Vector2 spriteTranslate_;
	// ロックオン距離の限界値
	const float kDistanceLockOn = 50.0f;
	// ロックオン対象リスト
	std::list<std::pair<float, BaseEnemy*>> targets;
	// ロックオン対象
	BaseEnemy* target_ = nullptr;
};

