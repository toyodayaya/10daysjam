#include "Player.h"
#include "Object3dCommon.h"
#include "ModelManager.h"
#include "Model.h"
#include "TextureManager.h"
#include "ImGuiManager.h"
#include "Input.h"
#include "EnemyManager.h"
#include "EventManager.h"
#include "LightHouse.h"
#include "SceneManager.h"
#include <algorithm>
#include <limits>
#include "DamageManager.h"

void Player::Initialize(const QuaternionTransform& transform, const std::string& filePath, bool isRailCamera)
{
	// オブジェクトの初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(Object3dCommon::GetInstance());
	object3d_->SetModel(filePath);
	object3d_->SetEnvironmentMapTextureFilePath("resources/human/white.png");
	object3d_->SetTransform(transform);
	object3d_->SetOffset(Vector3{ 0.0f,0.0f,10.0f });
	transform_ = transform;
	baseScale_ = transform.scale;
	respawnPosition_ = transform.translate;
	initialRespawnPosition_ = transform.translate;
	explosion_.Initialize();
	hpUI_.Initialize();
	hpUI_.Update(hp_, maxHp_);
	isHit_ = false;

	isDead_ = false;
}

void Player::Update()
{
	// 爆発判定は発生したフレームだけ有効にする
	explosion_.Deactivate();

	// HP自動回復処理
	AutoRecoveryHp();

	// 移動処理
	Move();
	// リスポーン時の出現演出を更新
	UpdateRespawnScaleAnimation();

	object3d_->SetTranslate(transform_.translate);
	object3d_->SetScale(transform_.scale);

	// 3Dオブジェクトを更新
	object3d_->Update();

	// 灯台本体の衝突判定とは独立して、インタラクト範囲を判定する
	UpdateLightHouseInteraction();

	// 自爆処理
	SelfDestruct();
	// 爆発範囲表示の更新
	explosion_.Update();
	// このフレームで変化した現在HP・最大HPをUIへ反映する
	hpUI_.Update(hp_, maxHp_);


#ifdef USE_IMGUI
	ImGui::Begin("Player");
	Vector3 transform = object3d_->GetWorldTranslate();
	ImGui::DragFloat3("pos", &transform.x);
	// HP表示
	ImGui::Text("HP: %d / %d", hp_, maxHp_);

	ImGui::End();

#endif // USE_IMGUI

	if (isDead_)
	{
		SceneManager::GetInstance()->ChangeScene("TitleScene");
	}
}

void Player::Draw()
{
	// 当たっていたら非表示にする
	if (isHit_)
	{
		return;
	}

	object3d_->Draw();
	// 爆発範囲の描画
	explosion_.Draw();
}

void Player::DrawUI()
{
	// UIはStageDataから全3Dオブジェクトの描画後に呼び出される
	hpUI_.Draw();
}

void Player::Finalize()
{}

void Player::OnCollision(std::string hitObjectType, BaseCharacter* hitObject)
{
	// ぶつかったオブジェクトのポインタを取得
	hitObject_ = hitObject;

	// ぶつかったオブジェクトのタイプを記録
	hitObjectType_ = hitObjectType;

	// どのオブジェクトにぶつかったか判定
	if (hitObjectType_ == "EventSpawn")
	{
		// 灯台だった場合の処理
		LightHouse* lightHouse = dynamic_cast<LightHouse*>(hitObject_);
		if (lightHouse == nullptr)
		{
			return;
		}

		// 灯台本体との重なりがなくなる位置までPlayerを押し戻す
		ResolveObstacleOverlap(lightHouse->GetCollisionAabb());
	}
	else if (hitObjectType_ == "EnemySpawn")
	{
		BaseEnemy* enemy = dynamic_cast<BaseEnemy*>(hitObject_);
		if (enemy != nullptr && !enemy->IsDead())
		{
			// Enemy本体との重なりがなくなる位置までPlayerを押し戻す
			ResolveObstacleOverlap(enemy->GetDamageAabb());
		}
	}

}

void Player::Move() {
	// キー入力でプレイヤーを移動させる
	Vector3 moveDirection = { 0.0f, 0.0f, 0.0f };

	if (Input::GetInstance()->PushKey(DIK_A))
	{
		moveDirection.x -= 1.0f;
	}
	if (Input::GetInstance()->PushKey(DIK_D))
	{
		moveDirection.x += 1.0f;
	}
	if (Input::GetInstance()->PushKey(DIK_S))
	{
		moveDirection.z -= 1.0f;
	}
	if (Input::GetInstance()->PushKey(DIK_W))
	{
		moveDirection.z += 1.0f;
	}

	constexpr float kMoveSpeed = 0.1f;
	moveDirection = MathManager::Normalize(moveDirection);
	transform_.translate.x += moveDirection.x * kMoveSpeed;
	transform_.translate.z += moveDirection.z * kMoveSpeed;

	// 範囲を超えない処理
	transform_.translate.x = max(transform_.translate.x, -kMoveLimitX_);
	transform_.translate.x = std::min(transform_.translate.x, +kMoveLimitX_);
	transform_.translate.z = max(transform_.translate.z, -kMoveLimitZ_);
	transform_.translate.z = std::min(transform_.translate.z, +kMoveLimitZ_);

}

void Player::SelfDestruct() {
	// スペースキーで自爆する
	if (Input::GetInstance()->TriggerKey(DIK_SPACE))
	{
		// リスポーン前の座標と残りHPを爆発へ反映する
		const Vector3 explosionCenter = object3d_->GetWorldTranslate();
		const int remainingHp = (std::max)(0, hp_);
		const int explosionDamage = kBaseExplosionDamage_ + remainingHp;
		explosion_.SetDamage(explosionDamage);

		// リスポーンでHPが変わる前に爆発の情報を確定させる
		Respawn();
		explosion_.Activate(explosionCenter);

		// 爆発が有効な発生フレームに一度だけEnemyへダメージを与える
		DamageEnemiesWithExplosion();
	}
}

void Player::DamageEnemiesWithExplosion()
{
	if (!explosion_.IsActive())
	{
		return;
	}

	// Enemy一覧を直接走査
	const std::vector<std::unique_ptr<BaseEnemy>>& enemies = EnemyManager::GetInstance()->GetEnemies();
	for (const std::unique_ptr<BaseEnemy>& enemy : enemies)
	{
		if (enemy->IsDead())
		{
			continue;
		}

		if (explosion_.IsCollision(enemy->GetDamageAabb()))
		{
			enemy->TakeDamage(explosion_.GetDamage());
		}
	}
}

void Player::ResolveObstacleOverlap(const AABB& obstacleAabb)
{
	// Playerの現在座標からAABBを作成する
	const AABB playerAabb = {
		{
			transform_.translate.x - kCollisionAabbHalfSize_.x,
			transform_.translate.y - kCollisionAabbHalfSize_.y,
			transform_.translate.z - kCollisionAabbHalfSize_.z
		},
		{
			transform_.translate.x + kCollisionAabbHalfSize_.x,
			transform_.translate.y + kCollisionAabbHalfSize_.y,
			transform_.translate.z + kCollisionAabbHalfSize_.z
		}
	};

	const float overlapX = (std::min)(playerAabb.max.x, obstacleAabb.max.x) -
		(std::max)(playerAabb.min.x, obstacleAabb.min.x);
	const float overlapY = (std::min)(playerAabb.max.y, obstacleAabb.max.y) -
		(std::max)(playerAabb.min.y, obstacleAabb.min.y);
	const float overlapZ = (std::min)(playerAabb.max.z, obstacleAabb.max.z) -
		(std::max)(playerAabb.min.z, obstacleAabb.min.z);

	// 3軸すべてが重なっている場合だけ押し戻す
	if (overlapX <= 0.0f || overlapY <= 0.0f || overlapZ <= 0.0f)
	{
		return;
	}

	const float obstacleCenterX = (obstacleAabb.min.x + obstacleAabb.max.x) * 0.5f;
	const float obstacleCenterZ = (obstacleAabb.min.z + obstacleAabb.max.z) * 0.5f;

	// X/Zのうち重なりが小さい軸へ押し戻すことで、障害物に沿って移動できるようにする
	if (overlapX <= overlapZ)
	{
		if (transform_.translate.x < obstacleCenterX)
		{
			transform_.translate.x -= overlapX;
		}
		else if (transform_.translate.x > obstacleCenterX)
		{
			transform_.translate.x += overlapX;
		}
		else
		{
			// 中心が一致するリスポーン時は、ステージ中央側へ押し出す
			transform_.translate.x += obstacleCenterX >= 0.0f ? -overlapX : overlapX;
		}
	}
	else
	{
		if (transform_.translate.z < obstacleCenterZ)
		{
			transform_.translate.z -= overlapZ;
		}
		else if (transform_.translate.z > obstacleCenterZ)
		{
			transform_.translate.z += overlapZ;
		}
		else
		{
			// 中心が一致する場合はステージ中央側へ押し出す
			transform_.translate.z += obstacleCenterZ >= 0.0f ? -overlapZ : overlapZ;
		}
	}

	// 衝突判定後の描画にも押し戻した座標を即時反映する
	object3d_->SetTranslate(transform_.translate);
	object3d_->Update();
}

void Player::UpdateLightHouseInteraction()
{
	LightHouse* targetLightHouse = nullptr;
	float nearestDistanceSquared = (std::numeric_limits<float>::max)();

	// 登録中の灯台を調べ、インタラクト範囲内で最も近い1基を選ぶ
	const std::vector<std::unique_ptr<BaseEvent>>& events = EventManager::GetInstance()->GetEvents();
	for (const std::unique_ptr<BaseEvent>& event : events)
	{
		LightHouse* lightHouse = dynamic_cast<LightHouse*>(event.get());
		// 使用・破壊中の灯台とはインタラクトしない
		if (lightHouse == nullptr || lightHouse->IsHit())
		{
			continue;
		}

		const AABB interactionAabb = lightHouse->GetInteractionAabb();
		const Vector3& playerPosition = transform_.translate;
		const bool isInsideInteractionAabb =
			playerPosition.x >= interactionAabb.min.x && playerPosition.x <= interactionAabb.max.x &&
			playerPosition.y >= interactionAabb.min.y && playerPosition.y <= interactionAabb.max.y &&
			playerPosition.z >= interactionAabb.min.z && playerPosition.z <= interactionAabb.max.z;

		if (!isInsideInteractionAabb)
		{
			continue;
		}

		const Vector3 lightHousePosition = lightHouse->GetTransform().translate;
		const float differenceX = playerPosition.x - lightHousePosition.x;
		const float differenceY = playerPosition.y - lightHousePosition.y;
		const float differenceZ = playerPosition.z - lightHousePosition.z;
		const float distanceSquared =
			differenceX * differenceX + differenceY * differenceY + differenceZ * differenceZ;

		if (distanceSquared < nearestDistanceSquared)
		{
			nearestDistanceSquared = distanceSquared;
			targetLightHouse = lightHouse;
		}
	}

	if (targetLightHouse == nullptr)
	{
		return;
	}

	// QキーでPlayerから灯台へHPを移す
	if (Input::GetInstance()->TriggerKey(DIK_Q) && hp_ > lighthouseHp_)
	{
		targetLightHouse->AddHP(static_cast<float>(lighthouseHp_));
		hp_ -= lighthouseHp_;
	}

	// ZキーでPlayerの最大HPを超えない範囲まで灯台のHPを回収する
	if (Input::GetInstance()->TriggerKey(DIK_Z))
	{
		const int receivableHp = maxHp_ - hp_;
		if (receivableHp > 0)
		{
			const uint32_t withdrawnHp =
				targetLightHouse->WithdrawHp(static_cast<uint32_t>(receivableHp));
			hp_ += static_cast<int>(withdrawnHp);
		}

		// ダメージを記録
		//DamageManager::GetInstance()->RankingUpdate(ダメージの数値);
	}
}

void Player::StartRespawnScaleAnimation()
{
	respawnScaleAnimationFrame_ = 0;
	isRespawnScaleAnimating_ = true;

	// 最初はほぼ見えない大きさにして、次フレーム以降で元の大きさへ戻す
	transform_.scale = {
		baseScale_.x * kRespawnStartScaleRate_,
		baseScale_.y * kRespawnStartScaleRate_,
		baseScale_.z * kRespawnStartScaleRate_
	};
}

void Player::UpdateRespawnScaleAnimation()
{
	if (!isRespawnScaleAnimating_)
	{
		return;
	}

	const float progress = static_cast<float>(respawnScaleAnimationFrame_) /
		static_cast<float>(kRespawnScaleAnimationFrames_ - 1);

	// EaseOutBackで一度少し膨らませ、元サイズへ戻すことで「にゅっ」とした動きにする
	constexpr float kBackStrength = 1.70158f;
	constexpr float kBackCoefficient = kBackStrength + 1.0f;
	const float offsetProgress = progress - 1.0f;
	const float easedProgress = 1.0f +
		kBackCoefficient * offsetProgress * offsetProgress * offsetProgress +
		kBackStrength * offsetProgress * offsetProgress;
	const float scaleRate = kRespawnStartScaleRate_ +
		(1.0f - kRespawnStartScaleRate_) * easedProgress;

	transform_.scale = {
		baseScale_.x * scaleRate,
		baseScale_.y * scaleRate,
		baseScale_.z * scaleRate
	};

	++respawnScaleAnimationFrame_;
	if (respawnScaleAnimationFrame_ >= kRespawnScaleAnimationFrames_)
	{
		// 誤差が残らないよう、終了時は本来のスケールを直接設定する
		transform_.scale = baseScale_;
		isRespawnScaleAnimating_ = false;
	}
}

// HP自動回復処理
void Player::AutoRecoveryHp() {

	// 1秒に1HPを回復する
	// HPが最大値に達していない場合のみ回復する
	if (hp_ < maxHp_) {
		// タイマーが1秒以上経過した場合に回復する
		if (recoveryHpTimer_ >= 1.0f) {
			hp_ += kRecoveryHp_;
			// 最大HPを超えないようにする
			hp_ = std::clamp(hp_, 0, maxHp_);
			recoveryHpTimer_ = 0.0f;
		}
	}

	// タイマーの加算
	recoveryHpTimer_ += 1.0f / 60.0f; // 60FPS
}

// リスポーン処理
void Player::Respawn() {
	explosion_.Deactivate();
	// 使用できる灯台がない場合は初期スポーン地点へ戻る
	respawnPosition_ = initialRespawnPosition_;

	// 保有中HPが最も多い灯台をリスポーン先にする
	LightHouse* respawnLightHouse = EventManager::GetInstance()->GetHighestHpLightHouse();
	if (respawnLightHouse != nullptr)
	{
		respawnPosition_ = respawnLightHouse->GetTransform().translate;
		// 灯台の保有HPが少なくても、Playerの最大HPは最低値を下回らない
		SetMaxHP(static_cast<float>(respawnLightHouse->GetHp()));
		respawnLightHouse->SetIsHit(true);
	}

	transform_.translate = respawnPosition_;
	StartRespawnScaleAnimation();
	object3d_->SetTranslate(transform_.translate);
	object3d_->SetScale(transform_.scale);
	// リスポーンしたフレームの描画にも座標と小さいスケールを反映する
	object3d_->Update();
	isDead_ = false;
	isHit_ = false;
}

void Player::AddHP(const float& hp)
{}

void Player::SetMaxHP(const float& hp)
{
	// 外部から10未満の値が渡されても、最大HPの最低値を保証する
	maxHp_ = (std::max)(kMinimumMaxHp_, static_cast<int>(hp));
	// 最大HPが現在HPより小さくなった場合は現在HPも上限内に収める
	hp_ = (std::min)(hp_, maxHp_);
}
