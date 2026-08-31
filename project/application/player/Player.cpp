#include "Player.h"
#include "Object3dCommon.h"
#include "ModelManager.h"
#include "Model.h"
#include "TextureManager.h"
#include "ImGuiManager.h"
#include "Input.h"
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
	isHit_ = false;

	isDead_ = false;
}

void Player::Update()
{
	// キー入力でプレイヤーを移動させる
	if (Input::GetInstance()->PushKey(DIK_A))
	{
		transform_.translate.x -= 0.1f;
	}
	else if (Input::GetInstance()->PushKey(DIK_D))
	{
		transform_.translate.x += 0.1f;
	}
	else if (Input::GetInstance()->PushKey(DIK_S))
	{
		transform_.translate.y -= 0.1f;
	}
	else if (Input::GetInstance()->PushKey(DIK_W))
	{
		transform_.translate.y += 0.1f;
	}

	// 範囲を超えない処理
	transform_.translate.x = max(transform_.translate.x, -kMoveLimitX_);
	transform_.translate.x = std::min(transform_.translate.x, +kMoveLimitX_);
	transform_.translate.y = max(transform_.translate.y, -kMoveLimitY_);
	transform_.translate.y = std::min(transform_.translate.y, +kMoveLimitY_);


	object3d_->SetTranslate(transform_.translate);

	// 3Dオブジェクトを更新
	object3d_->Update();


#ifdef USE_IMGUI
	ImGui::Begin("Player");
	Vector3 transform = object3d_->GetWorldTranslate();
	ImGui::DragFloat3("pos", &transform.x);

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
}

void Player::Finalize()
{}

void Player::OnCollision(std::string hitObjectType)
{
	// ぶつかったオブジェクトのタイプを記録
	hitObjectType_ = hitObjectType;

	// どのオブジェクトにぶつかったか判定
	if (hitObjectType_ == "EventSpawn")
	{
		// 灯台だった場合の処理
	}
	else if (hitObjectType_ == "EnemySpawn")
	{
		// ボスだった場合の処理
	}

	isDead_ = true;
}
