#include "Player.h"
#include "Object3dCommon.h"
#include "ModelManager.h"
#include "Model.h"
#include "TextureManager.h"
#include "ImGuiManager.h"
#include "Input.h"
#include "EventManager.h"
#include "LightHouse.h"
#include "SceneManager.h"

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
	respawnPosition_ = transform.translate;
	explosion_.Initialize();
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

	object3d_->SetTranslate(transform_.translate);

	// 3Dオブジェクトを更新
	object3d_->Update();

	// 自爆処理
	SelfDestruct();
	// 爆発範囲表示の更新
	explosion_.Update();


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

		// EキーでHPを移す
		if (Input::GetInstance()->TriggerKey(DIK_Q))
		{
			// HPを移す処理
			// 灯台に移すHPが残っていたら移せる
			if (hp_ > lighthouseHp_)
			{
				// 衝突相手が灯台の場合のみHPを移す
				LightHouse* lightHouse = dynamic_cast<LightHouse*>(hitObject_);
				if (lightHouse != nullptr)
				{
					lightHouse->AddHP(static_cast<float>(lighthouseHp_));
					hp_ -= lighthouseHp_;
				}
			}
		}
	}
	else if (hitObjectType_ == "EnemySpawn")
	{
		// ボスだった場合の処理
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
		// リスポーン前のワールド座標を爆心地として保存する
		const Vector3 explosionCenter = object3d_->GetWorldTranslate();
		Respawn();
		explosion_.Activate(explosionCenter);
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

	// 保有中HPが最も多い灯台をリスポーン先にする
	LightHouse* respawnLightHouse = EventManager::GetInstance()->GetHighestHpLightHouse();
	if (respawnLightHouse != nullptr)
	{
		respawnPosition_ = respawnLightHouse->GetTransform().translate;
		maxHp_ = static_cast<int>(respawnLightHouse->GetHp());
		hp_ = std::min(hp_, maxHp_);
	}

	transform_.translate = respawnPosition_;
	object3d_->SetTranslate(transform_.translate);
	isDead_ = false;
	isHit_ = false;
}

void Player::AddHP(const float& hp)
{}

void Player::SetMaxHP(const float& hp)
{}
