#pragma once
#include "BaseEnemy.h"
#include "Object3d.h"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

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
	void OnCollision(std::string hitObjectType, BaseCharacter* hitObject) override;

	void TakeDamage(int damage);

	// getter
	Vector3 GetTranslate() { return transform_.translate; }
	// HP加算関数
	void AddHP(const float& hp) override;
	// 最大HPを設定
	void SetMaxHP(const float& hp) override;

private:
	// 巡回射撃の後、地面叩きつけと灯台突進を交互に使う。
	enum class AttackState
	{
		Patrol, // 上から見て横8の字になるように移動する
		Charge, // 狙う位置を決めて、ためる
		Rush,   // 決めた位置に向かって突進する
		Return, // 最初に配置された位置へ戻る
		SlamCharge, // 地面叩きつけの予告
		SlamApproach, // プレイヤーの真上へ移動
		SlamHover,  // 真上で停止し、落下位置を見せる
		SlamFall,   // 地面へ急降下
		SlamImpact, // 着地して衝撃を出す
		SlamReturn, // 叩きつけ後に初期位置へ戻る
		Recover, // 初期位置で止まり、自爆を狙える隙を作る
	};

	void UpdateAttack();
	void UpdatePatrolMovement();
	bool TryStartSpecialAttack();
	bool TryStartLighthouseAttack();
	void UpdatePatrolShooting();
	bool TryShootAtPlayer();
	bool CreateBullet(const Vector3& direction, float spawnOffset);
	void UpdateBullets();
	bool TryStartSlamAttack();
	void ResetSlamScale();

	// 弾はEnemyが所有する。現段階では移動・描画のみで、当たり判定は持たない。
	struct Bullet
	{
		Vector3 position = { 0.0f, 0.0f, 0.0f };
		Vector3 velocity = { 0.0f, 0.0f, 0.0f };
		int remainingFrames = 0;
		std::unique_ptr<Object3d> object3d;
	};
	static constexpr int kShotIntervalFrames_ = 60;   // 巡回中、約1秒ごとに発射
	static constexpr int kBulletLifetimeFrames_ = 240; // 弾の寿命：4秒
	static constexpr float kBulletSpeed_ = 0.15f;      // 弾の1フレームの移動量
	static constexpr float kBulletScale_ = 0.15f;      // 仮表示：ボスの15%の大きさ
	static constexpr float kBulletSpawnOffset_ = 0.7f; // ボスの中心より少し前から発射
	int shotTimer_ = kShotIntervalFrames_;
	std::string bulletModelFilePath_;
	std::vector<Bullet> bullets_;

	// 狙いを決める時、このHP（明るさ）以上の灯台だけを対象にする。
	static constexpr uint32_t kTargetMinHp_ = 5;

	// Playerと同じく60FPSを前提にした調整値。
	static constexpr int kPatrolFrames_ = 180;       // 巡回してから灯台を狙う：3秒
	static constexpr int kChargeFrames_ = 45;        // ため：0.75秒
	static constexpr int kRecoveryFrames_ = 60;      // 帰還後の隙：1秒
	static constexpr int kSlamChargeFrames_ = 60;    // その場での予告：1秒
	static constexpr int kSlamApproachFrames_ = 45;  // プレイヤーの真上へ移動：0.75秒
	static constexpr int kSlamHoverFrames_ = 45;     // 真上で停止して着地点を予告：0.75秒
	static constexpr int kSlamFallFrames_ = 18;      // 急降下：0.3秒
	static constexpr int kSlamImpactFrames_ = 24;    // 着地時の衝撃表示：0.4秒
	static constexpr float kRushSpeed_ = 0.2f;        // 1フレームの移動量
	static constexpr float kReturnSpeed_ = 0.1f;      // 戻るときの1フレームの移動量
	static constexpr float kSlamJumpHeight_ = 3.0f;   // 叩きつけで跳び上がる高さ
	static constexpr float kSlamChargeScale_ = 1.20f; // 予告中の最大拡大率
	static constexpr float kSlamImpactScale_ = 1.45f; // 着地時の最大拡大率
	// 初期位置を中心とする巡回範囲。画面・ステージに合わせて調整する。
	static constexpr float kPatrolRadiusX_ = 2.5f;   // 左右に最大2.5
	static constexpr float kPatrolRadiusZ_ = 1.0f;   // 奥行きに最大1.0
	static constexpr int kPatrolCycleFrames_ = 240;  // 横8の字を1周する時間：4秒

	AttackState attackState_ = AttackState::Patrol;
	int attackTimer_ = kPatrolFrames_;
	bool nextAttackIsSlam_ = true; // 最初の特殊攻撃はプレイヤーへの叩きつけ
	int patrolFrame_ = 0;
	float patrolDirection_ = 1.0f; // 帰還するたびに左右を反転する
	// Initializeで保存する初期位置（transform_と同じローカル座標）。
	Vector3 startPosition_ = { 0.0f, 0.0f, 0.0f };
	Vector3 startScale_ = { 1.0f, 1.0f, 1.0f };
	// 灯台のポインタを保持せず、攻撃開始時の位置だけを保存する。
	Vector3 attackTargetPosition_ = { 0.0f, 0.0f, 0.0f };
	// 叩きつけ開始時と、攻撃開始時に固定したプレイヤー位置。
	Vector3 slamStartPosition_ = { 0.0f, 0.0f, 0.0f };
	Vector3 slamTargetPosition_ = { 0.0f, 0.0f, 0.0f };

	// ボスのHP（最初は動作確認用に10）
	const int kMaxHp_ = 10;
	int hp_ = kMaxHp_;
};

