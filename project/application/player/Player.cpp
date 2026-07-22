#include "Player.h"
#include "Object3dCommon.h"
#include "ModelManager.h"
#include "Model.h"
#include "TextureManager.h"
#include "ImGuiManager.h"

void Player::Initialize(QuaternionTransform transform, const std::string& filePath)
{
	// オブジェクトの初期化
	object3d = std::make_unique<Object3d>();
	object3d->Initialize(Object3dCommon::GetInstance());
	object3d->SetModel(filePath);
	object3d->SetEnvironmentMapTextureFilePath("resources/human/white.png");
	object3d->SetTransform(transform);
}

void Player::Update()
{
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
	object3d->Draw();
}
