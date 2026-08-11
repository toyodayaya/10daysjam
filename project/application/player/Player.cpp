#include "Player.h"
#include "Object3dCommon.h"
#include "ModelManager.h"
#include "Model.h"
#include "TextureManager.h"
#include "ImGuiManager.h"
#include "Input.h"
#include "BulletManager.h"

void Player::Initialize(const QuaternionTransform& transform, const std::string& filePath, bool isRailCamera)
{
	// オブジェクトの初期化
	object3d = std::make_unique<Object3d>();
	object3d->Initialize(Object3dCommon::GetInstance());
	object3d->SetModel(filePath);
	object3d->SetEnvironmentMapTextureFilePath("resources/human/white.png");
	object3d->SetTransform(transform);
	object3d->SetIsRailCamera(isRailCamera);
	object3d->SetOffset(Vector3{ 0.0f,0.0f,10.0f });
	transform_ = transform;
	isHit_ = false;
}

void Player::Update()
{
	// キー入力で弾を生成
	if (Input::GetInstance()->TriggerKey(DIK_SPACE))
	{
		CreateBullet();
	}

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


	object3d->SetTranslate(transform_.translate);

	// 3Dオブジェクトを更新
	object3d->Update();

#ifdef USE_IMGUI
	ImGui::Begin("Player");
	QuaternionTransform transform = object3d->GetTransform();
	ImGui::DragFloat3("pos", &transform.translate.x);
	ImGui::DragFloat3("scale", &transform.scale.x);
	ImGui::DragFloat4("rotate", &transform.rotate.x);

	ImGui::End();

#endif // USE_IMGUI

}

void Player::Draw()
{
	// 当たっていたら非表示にする
	if (isHit_)
	{
		return;
	}

	object3d->Draw();
}

void Player::Finalize()
{}

void Player::OnCollision()
{
	isHit_ = true;
}

void Player::CreateBullet()
{
	// 速度を算出
	Vector3 velocity(0.0f, 0.0f, kBulletSpeed_);
	Matrix4x4 world = MakeAffineMatrixQuat(transform_.scale, transform_.rotate, transform_.translate);
	velocity = TransformNormal(velocity, world);

	// 生成と初期化
	std::unique_ptr<Bullet> bullet = std::make_unique<Bullet>();
	bullet->Initialize(transform_, filePath,velocity);
	BulletManager::GetInstance()->SetBullets(std::move(bullet));
}
