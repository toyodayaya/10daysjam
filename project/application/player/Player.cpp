#include "Player.h"
#include "Object3dCommon.h"
#include "ModelManager.h"
#include "Model.h"
#include "TextureManager.h"
#include "ImGuiManager.h"
#include "Input.h"

void Player::Initialize(const QuaternionTransform& transform, const std::string& filePath)
{
	// オブジェクトの初期化
	object3d = std::make_unique<Object3d>();
	object3d->Initialize(Object3dCommon::GetInstance());
	object3d->SetModel(filePath);
	object3d->SetEnvironmentMapTextureFilePath("resources/human/white.png");
	object3d->SetTransform(transform);
	transform_ = transform;
	isHit_ = false;
}

void Player::Update()
{

	// キー入力でプレイヤーを移動させる
	if (Input::GetInstance()->PushKey(DIK_A))
	{
		transform_.translate.x -= 0.1f;
	}

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
