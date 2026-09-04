#include "Enemy.h"
#include "Object3dCommon.h"
#ifdef USE_IMGUI
#include "ImGuiManager.h"
#endif // USE_IMGUI
#include "Player.h"
#include "EventManager.h"
#include "LightHouse.h"
#include "CollisionManager.h"
#include <cmath>
#include <utility>

void Enemy::Initialize(const QuaternionTransform& transform, const std::string& filePath)
{
	// 3Dオブジェクトを初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(Object3dCommon::GetInstance());
	object3d_->SetModel(filePath);
	object3d_->SetEnvironmentMapTextureFilePath("resources/human/white.png");
	object3d_->SetTransform(transform);
	transform_ = transform;
	hp_ = kMaxHp_;
	isDead_ = false;
	attackState_ = AttackState::Patrol;
	attackTimer_ = kPatrolFrames_;
	nextAttackIsSlam_ = true;
	patrolFrame_ = 0;
	patrolDirection_ = 1.0f;
	startPosition_ = transform_.translate;
	startScale_ = transform_.scale;
	attackTargetPosition_ = transform_.translate;
	slamStartPosition_ = transform_.translate;
	slamTargetPosition_ = transform_.translate;
	shotTimer_ = kShotIntervalFrames_;
	bulletModelFilePath_ = filePath; // 読み込み済みのボスモデルを弾の仮表示に再利用。
	bullets_.clear();
}

void Enemy::Finalize()
{
	bullets_.clear();
}

void Enemy::Update()
{
	if (isDead_)
	{
		return;
	}

	UpdateAttack();
	// 移動と、地面叩きつけ中の大きさの変化を描画へ反映する。
	object3d_->SetTransform(transform_);
	object3d_->Update();
	// 既に出ている弾は状態に関係なく進む。新しい弾は巡回中だけ発射する。
	UpdateBullets();
	UpdatePatrolShooting();

#ifdef USE_IMGUI
	ImGui::Begin("Boss");
	ImGui::Text("HP: %d / %d", hp_, kMaxHp_);
	ImGui::Text("Bullets: %d / Shot timer: %d",
		static_cast<int>(bullets_.size()), shotTimer_);
	const char* attackStateName = "Patrol";
	if (attackState_ == AttackState::Charge)
	{
		attackStateName = "Charge";
	}
	else if (attackState_ == AttackState::Rush)
	{
		attackStateName = "Rush";
	}
	else if (attackState_ == AttackState::Return)
	{
		attackStateName = "Return";
	}
	else if (attackState_ == AttackState::SlamCharge)
	{
		attackStateName = "SlamCharge";
	}
	else if (attackState_ == AttackState::SlamApproach)
	{
		attackStateName = "SlamApproach";
	}
	else if (attackState_ == AttackState::SlamHover)
	{
		attackStateName = "SlamHover";
	}
	else if (attackState_ == AttackState::SlamFall)
	{
		attackStateName = "SlamFall";
	}
	else if (attackState_ == AttackState::SlamImpact)
	{
		attackStateName = "SlamImpact";
	}
	else if (attackState_ == AttackState::SlamReturn)
	{
		attackStateName = "SlamReturn";
	}
	else if (attackState_ == AttackState::Recover)
	{
		attackStateName = "Recover";
	}
	ImGui::Text("Attack: %s / Timer: %d", attackStateName, attackTimer_);
	if (attackState_ == AttackState::Return)
	{
		ImGui::Text("Return XZ: %.2f, %.2f", startPosition_.x, startPosition_.z);
	}
	else if (attackState_ == AttackState::Charge || attackState_ == AttackState::Rush)
	{
		ImGui::Text("Target XZ: %.2f, %.2f",
			attackTargetPosition_.x, attackTargetPosition_.z);
	}
	else if (attackState_ == AttackState::SlamCharge ||
		attackState_ == AttackState::SlamApproach ||
		attackState_ == AttackState::SlamHover ||
		attackState_ == AttackState::SlamFall ||
		attackState_ == AttackState::SlamImpact)
	{
		ImGui::Text("Slam target XZ: %.2f, %.2f",
			slamTargetPosition_.x, slamTargetPosition_.z);
	}
	// 攻撃判定を接続する前でも、HP減少と撃破を確認できる。
	if (ImGui::Button("Take 1 damage (debug)"))
	{
		TakeDamage(1);
	}
	ImGui::End();
#endif // USE_IMGUI
}

bool Enemy::TryStartSpecialAttack()
{
	if (nextAttackIsSlam_)
	{
		if (TryStartSlamAttack())
		{
			nextAttackIsSlam_ = false;
			return true;
		}
		// プレイヤーが見つからない場合は、灯台への攻撃を試す。
		if (TryStartLighthouseAttack())
		{
			nextAttackIsSlam_ = true;
			return true;
		}
	}
	else
	{
		if (TryStartLighthouseAttack())
		{
			nextAttackIsSlam_ = true;
			return true;
		}
		// 明るい灯台がない場合も、プレイヤーへの攻撃は止めない。
		if (TryStartSlamAttack())
		{
			nextAttackIsSlam_ = false;
			return true;
		}
	}
	return false;
}

bool Enemy::TryStartLighthouseAttack()
{
	// Playerのリスポーン先選択にも使われている既存の関数を再利用。
	LightHouse* lightHouse = EventManager::GetInstance()->GetHighestHpLightHouse();
	if (!lightHouse || lightHouse->IsDead() || lightHouse->GetHp() < kTargetMinHp_)
	{
		// 最も明るい灯台でもしきい値未満なら、対象はないので巡回を続ける。
		return false;
	}

	// この時点で狙いを固定。後から明るさが変わっても中断・追い直しはしない。
	attackTargetPosition_ = lightHouse->GetObject3d()->GetWorldTranslate();
	attackState_ = AttackState::Charge;
	attackTimer_ = kChargeFrames_;
	return true;
}

void Enemy::UpdatePatrolMovement()
{
	// 角度を際限なく増やさず、1周分のフレーム数で折り返す。
	patrolFrame_ = (patrolFrame_ + 1) % kPatrolCycleFrames_;
	constexpr float kTwoPi = 6.28318530718f;
	const float phase = kTwoPi * static_cast<float>(patrolFrame_)
		/ static_cast<float>(kPatrolCycleFrames_);

	// XとZで周期を変えると、俯瞰で横8の字になる。Yは動かさない。
	// 速度を毎回加算せず、初期位置からの差で計算するため中心がずれない。
	transform_.translate.x = startPosition_.x
		+ patrolDirection_ * kPatrolRadiusX_ * std::sin(phase);
	transform_.translate.z = startPosition_.z
		+ kPatrolRadiusZ_ * std::sin(phase * 2.0f);
}

void Enemy::UpdateAttack()
{
	switch (attackState_)
	{
	case AttackState::Patrol:
		UpdatePatrolMovement();
		if (attackTimer_ > 0)
		{
			--attackTimer_;
		}
		if (attackTimer_ == 0)
		{
			if (!TryStartSpecialAttack())
			{
				// 攻撃対象がない場合も、毎フレーム検索せず次の巡回後に再試行する。
				attackTimer_ = kPatrolFrames_;
			}
		}
		break;

	case AttackState::Charge:
		// ため中は移動しない。次のフレームから突進を開始する。
		if (--attackTimer_ <= 0)
		{
			attackTimer_ = 0;
			attackState_ = AttackState::Rush;
		}
		break;

	case AttackState::Rush:
	{
		// プレイヤーの移動と同じXZ平面で突進し、高さは変えない。
		const Vector3 currentPosition = object3d_->GetWorldTranslate();
		const float dx = attackTargetPosition_.x - currentPosition.x;
		const float dz = attackTargetPosition_.z - currentPosition.z;
		const float distance = std::sqrt(dx * dx + dz * dz);

		if (distance <= kRushSpeed_)
		{
			// 目的地を通り越さない。距離0でも割り算をしない。
			transform_.translate.x += dx;
			transform_.translate.z += dz;
			// 灯台に到着したら、次のフレームから初期位置へ戻る。
			attackState_ = AttackState::Return;
			attackTimer_ = 0;
		}
		else
		{
			transform_.translate.x += dx / distance * kRushSpeed_;
			transform_.translate.z += dz / distance * kRushSpeed_;
		}
		// 灯台のHPはここで直接減らさない。
		// 既存のStageDataの接触判定からLightHouse::OnCollision()が呼ばれ、
		// 灯台側の減衰処理によって明るさ（HP）が減る。
		break;
	}

	case AttackState::Return:
	{
		// 戻っている間は灯台を選び直さず、保存済みの初期位置へ移動する。
		const float dx = startPosition_.x - transform_.translate.x;
		const float dz = startPosition_.z - transform_.translate.z;
		const float distance = std::sqrt(dx * dx + dz * dz);
		if (distance <= kReturnSpeed_)
		{
			// 灯台への突進後は叩きつけへ直結せず、一度隙を作る。
			transform_.translate = startPosition_;
			attackState_ = AttackState::Recover;
			attackTimer_ = kRecoveryFrames_;
		}
		else
		{
			transform_.translate.x += dx / distance * kReturnSpeed_;
			transform_.translate.z += dz / distance * kReturnSpeed_;
		}
		break;
	}

	case AttackState::SlamCharge:
	{
		// 1秒かけて2回脈打ち、これから叩きつけることを強く予告する。
		constexpr float kPi = 3.14159265359f;
		const float progress = 1.0f - static_cast<float>(attackTimer_)
			/ static_cast<float>(kSlamChargeFrames_);
		const float pulse = 0.5f - 0.5f * std::cos(kPi * 4.0f * progress);
		const float scaleRate = 1.0f
			+ (kSlamChargeScale_ - 1.0f) * pulse;
		transform_.scale = {
			startScale_.x * scaleRate,
			startScale_.y * scaleRate,
			startScale_.z * scaleRate
		};

		if (--attackTimer_ <= 0)
		{
			ResetSlamScale();
			slamStartPosition_ = transform_.translate;
			attackState_ = AttackState::SlamApproach;
			attackTimer_ = kSlamApproachFrames_;
		}
		break;
	}

	case AttackState::SlamApproach:
	{
		// 高く上がりながら、攻撃開始時に固定したプレイヤーの真上へ移動する。
		constexpr float kPi = 3.14159265359f;
		const int elapsedFrames = kSlamApproachFrames_ - attackTimer_ + 1;
		const float progress = static_cast<float>(elapsedFrames)
			/ static_cast<float>(kSlamApproachFrames_);
		transform_.translate.x = slamStartPosition_.x
			+ (slamTargetPosition_.x - slamStartPosition_.x) * progress;
		transform_.translate.z = slamStartPosition_.z
			+ (slamTargetPosition_.z - slamStartPosition_.z) * progress;
		transform_.translate.y = slamStartPosition_.y
			+ std::sin(kPi * 0.5f * progress) * kSlamJumpHeight_;

		if (--attackTimer_ <= 0)
		{
			transform_.translate = {
				slamTargetPosition_.x,
				slamTargetPosition_.y + kSlamJumpHeight_,
				slamTargetPosition_.z
			};
			attackState_ = AttackState::SlamHover;
			attackTimer_ = kSlamHoverFrames_;
		}
		break;
	}

	case AttackState::SlamHover:
	{
		// プレイヤーの真上で0.75秒停止する。ここが避けるための一番分かりやすい猶予。
		constexpr float kPi = 3.14159265359f;
		const float progress = 1.0f - static_cast<float>(attackTimer_)
			/ static_cast<float>(kSlamHoverFrames_);
		const float pulse = 0.5f - 0.5f * std::cos(kPi * 6.0f * progress);
		const float scaleRate = 1.0f + 0.1f * pulse;
		transform_.scale = {
			startScale_.x * scaleRate,
			startScale_.y * scaleRate,
			startScale_.z * scaleRate
		};

		if (--attackTimer_ <= 0)
		{
			ResetSlamScale();
			attackState_ = AttackState::SlamFall;
			attackTimer_ = kSlamFallFrames_;
		}
		break;
	}

	case AttackState::SlamFall:
	{
		// 最初はゆっくり、地面に近づくほど速くなる落下にする。
		const int elapsedFrames = kSlamFallFrames_ - attackTimer_ + 1;
		const float progress = static_cast<float>(elapsedFrames)
			/ static_cast<float>(kSlamFallFrames_);
		const float acceleratedProgress = progress * progress;
		transform_.translate.x = slamTargetPosition_.x;
		transform_.translate.y = slamTargetPosition_.y
			+ kSlamJumpHeight_ * (1.0f - acceleratedProgress);
		transform_.translate.z = slamTargetPosition_.z;

		if (--attackTimer_ <= 0)
		{
			transform_.translate = slamTargetPosition_;
			attackState_ = AttackState::SlamImpact;
			attackTimer_ = kSlamImpactFrames_;
		}
		break;
	}

	case AttackState::SlamImpact:
	{
		// XZ方向へ一度だけ大きく広がり、着地の衝撃を表現する。
		constexpr float kPi = 3.14159265359f;
		const int elapsedFrames = kSlamImpactFrames_ - attackTimer_ + 1;
		const float progress = static_cast<float>(elapsedFrames)
			/ static_cast<float>(kSlamImpactFrames_);
		const float impactRate = std::sin(kPi * progress);
		const float horizontalScale = 1.0f
			+ (kSlamImpactScale_ - 1.0f) * impactRate;
		transform_.scale = {
			startScale_.x * horizontalScale,
			startScale_.y * (1.0f - 0.2f * impactRate),
			startScale_.z * horizontalScale
		};

		if (--attackTimer_ <= 0)
		{
			ResetSlamScale();
			attackState_ = AttackState::SlamReturn;
			attackTimer_ = 0;
		}
		break;
	}

	case AttackState::SlamReturn:
	{
		// 叩きつけた位置から最初の位置へ戻り、攻撃を終了する。
		const float dx = startPosition_.x - transform_.translate.x;
		const float dz = startPosition_.z - transform_.translate.z;
		const float distance = std::sqrt(dx * dx + dz * dz);
		if (distance <= kReturnSpeed_)
		{
			transform_.translate = startPosition_;
			attackState_ = AttackState::Recover;
			attackTimer_ = kRecoveryFrames_;
		}
		else
		{
			transform_.translate.x += dx / distance * kReturnSpeed_;
			transform_.translate.z += dz / distance * kReturnSpeed_;
		}
		break;
	}

	case AttackState::Recover:
		// この間はその場で停止。叩きつけ後に1秒の反撃チャンスを残す。
		if (--attackTimer_ <= 0)
		{
			attackState_ = AttackState::Patrol;
			attackTimer_ = kPatrolFrames_;
			patrolFrame_ = 0;
			patrolDirection_ *= -1.0f;
		}
		break;
	}
}

void Enemy::UpdatePatrolShooting()
{
	if (attackState_ != AttackState::Patrol)
	{
		// ため・突進・帰還・隙では発射しない。巡回再開時も発射間隔を確保する。
		shotTimer_ = kShotIntervalFrames_;
		return;
	}

	if (--shotTimer_ > 0)
	{
		return;
	}
	shotTimer_ = kShotIntervalFrames_;
	TryShootAtPlayer();
}

bool Enemy::TryShootAtPlayer()
{
	// 登録済みのプレイヤーを探すためだけに参照する。
	// 弾を登録したり、当たり判定やダメージ処理を呼んだりはしない。
	Player* targetPlayer = nullptr;
	for (const auto& collider : CollisionManager::GetInstance()->GetColliders())
	{
		if (collider.objectType != "PlayerSpawn" || !collider.parent)
		{
			continue;
		}
		auto* player = dynamic_cast<Player*>(collider.parent);
		if (player && !player->IsDead() && player->GetObject3d())
		{
			targetPlayer = player;
			break;
		}
	}
	if (!targetPlayer)
	{
		return false;
	}

	const Vector3 origin = object3d_->GetWorldTranslate();
	// Playerのオフセットも含むワールド座標を狙う。
	const Vector3 target = targetPlayer->GetObject3d()->GetWorldTranslate();
	const float dx = target.x - origin.x;
	const float dz = target.z - origin.z;
	const float distance = std::sqrt(dx * dx + dz * dz);
	if (distance <= 0.0001f)
	{
		// 同じ位置では方向を決められないので、今回の発射を見送る。
		return false;
	}
	const Vector3 direction = { dx / distance, 0.0f, dz / distance };
	const float spawnOffset = distance < kBulletSpawnOffset_ ? distance : kBulletSpawnOffset_;
	return CreateBullet(direction, spawnOffset);
}

bool Enemy::TryStartSlamAttack()
{
	// プレイヤーの現在位置を一度だけ記録し、跳んでいる途中では追尾しない。
	Player* targetPlayer = nullptr;
	for (const auto& collider : CollisionManager::GetInstance()->GetColliders())
	{
		if (collider.objectType != "PlayerSpawn" || !collider.parent)
		{
			continue;
		}
		auto* player = dynamic_cast<Player*>(collider.parent);
		if (player && !player->IsDead() && player->GetObject3d())
		{
			targetPlayer = player;
			break;
		}
	}
	if (!targetPlayer)
	{
		return false;
	}

	slamStartPosition_ = transform_.translate;
	slamTargetPosition_ = targetPlayer->GetObject3d()->GetWorldTranslate();
	// 着地する高さはボスの初期配置と同じにする。
	slamTargetPosition_.y = startPosition_.y;
	ResetSlamScale();
	attackState_ = AttackState::SlamCharge;
	attackTimer_ = kSlamChargeFrames_;
	return true;
}

void Enemy::ResetSlamScale()
{
	transform_.scale = startScale_;
}

bool Enemy::CreateBullet(const Vector3& direction, float spawnOffset)
{
	const float length = std::sqrt(
		direction.x * direction.x + direction.z * direction.z);
	if (length <= 0.0001f)
	{
		return false;
	}
	const Vector3 normalizedDirection = {
		direction.x / length, 0.0f, direction.z / length
	};
	const Vector3 origin = object3d_->GetWorldTranslate();

	Bullet bullet;
	bullet.position = {
		origin.x + normalizedDirection.x * spawnOffset,
		origin.y,
		origin.z + normalizedDirection.z * spawnOffset
	};
	bullet.velocity = {
		normalizedDirection.x * kBulletSpeed_,
		0.0f,
		normalizedDirection.z * kBulletSpeed_
	};
	bullet.remainingFrames = kBulletLifetimeFrames_;
	bullet.object3d = std::make_unique<Object3d>();
	bullet.object3d->Initialize(Object3dCommon::GetInstance());
	bullet.object3d->SetModel(bulletModelFilePath_);
	bullet.object3d->SetEnvironmentMapTextureFilePath("resources/human/white.png");
	QuaternionTransform bulletTransform = transform_;
	bulletTransform.translate = bullet.position;
	bulletTransform.scale = {
		transform_.scale.x * kBulletScale_,
		transform_.scale.y * kBulletScale_,
		transform_.scale.z * kBulletScale_
	};
	bullet.object3d->SetTransform(bulletTransform);
	bullet.object3d->Update();
	bullets_.push_back(std::move(bullet));
	return true;
}

void Enemy::UpdateBullets()
{
	for (auto it = bullets_.begin(); it != bullets_.end();)
	{
		if (--it->remainingFrames <= 0)
		{
			it = bullets_.erase(it);
			continue;
		}
		it->position.x += it->velocity.x;
		it->position.z += it->velocity.z;
		it->object3d->SetTranslate(it->position);
		it->object3d->Update();
		++it;
	}
}

void Enemy::Draw()
{
	if (isDead_)
	{
		return;
	}

	object3d_->Draw();
	for (const auto& bullet : bullets_)
	{
		bullet.object3d->Draw();
	}
}

void Enemy::OnCollision(std::string hitObjectType, BaseCharacter* hitObject)
{
	// 自爆が命中したという通知の場合
	if (hitObjectType == "Explosion")
	{
		Player* player = dynamic_cast<Player*>(hitObject);
		if (player == nullptr)
		{
			return;
		}

		const Explosion& explosion = player->GetExplosion();

		if (explosion.IsActive())
		{
			TakeDamage(explosion.GetDamage());
		}
	}
}

void Enemy::AddHP(const float& hp)
{}

void Enemy::SetMaxHP(const float& hp)
{}

void Enemy::TakeDamage(int damage)
{
	// 撃破後や、0以下のダメージではHPを変更しない。
	if (isDead_ || damage <= 0)
	{
		return;
	}

	// HPが負にならないようにする。
	hp_ = (damage >= hp_) ? 0 : hp_ - damage;
	if (hp_ == 0)
	{
		isDead_ = true;
		bullets_.clear(); // 撃破時に残弾も消す。
	}
	// クリアへの遷移はGamePlayScene側で行う。
}
