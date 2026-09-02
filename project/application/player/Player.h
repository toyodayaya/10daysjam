#pragma once
#include "Object3d.h"
#include "Sprite.h"
#include <memory>
#include "BaseCharacter.h"
#include "Explosion.h"

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
	void OnCollision(std::string hitObjectType, BaseCharacter* hitObject) override;

	// 移動処理
	void Move();

	// 自爆処理
	void SelfDestruct();

	// HP自動回復処理
	void AutoRecoveryHp();

	// リスポーン処理
	void Respawn();

	// 爆発攻撃を取得
	const Explosion& GetExplosion() const { return explosion_; }

	// HP加算関数
	void AddHP(const float& hp) override;

private:
	// 発生中の爆発とEnemyの当たり判定を行う
	void DamageEnemiesWithExplosion();
	// Playerと障害物のAABBの重なりを解消する
	void ResolveObstacleOverlap(const AABB& obstacleAabb);
	// 範囲内で最も近い灯台とのインタラクトを処理する
	void UpdateLightHouseInteraction();
	
	// 当たり判定フラグ
	bool isHit_;

	// リスポーン座標
	Vector3 respawnPosition_ = { 0.0f, 0.0f, 0.0f };

	// 自爆攻撃
	const float kExplosionRadius_ = 3.0f;
	const int kExplosionDamage_ = 10;
	Explosion explosion_{ kExplosionRadius_, kExplosionDamage_ };
	// Playerの当たり判定用AABBの中心から各面までの距離
	const Vector3 kCollisionAabbHalfSize_ = { 1.0f, 1.0f, 1.0f };
	
	// 移動限界
	const float kMoveLimitX_ = 7.0f;
	const float kMoveLimitZ_ = 7.0f;
	
	// 初期最大HP
	const int kIniMaxHp_ = 20;
	// 最大HP
	int maxHp_ = kIniMaxHp_;
	// HP
	int hp_ = kIniMaxHp_ / 2;
	// 1秒に回復するHP
	const int kRecoveryHp_ = 1;
	// HP回復タイマー(秒)
	float recoveryHpTimer_ = 0.0f;

	// 灯台にうつすHPの数値
	const int lighthouseHp_ = 5;
};

