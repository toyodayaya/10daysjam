#include "Skydome.h"
#include "ModelManager.h"
#include "Object3dCommon.h"

void Skydome::Initialize(const QuaternionTransform& transform, std::string textureFilePath)
{
	// 3Dオブジェクトの生成と初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(Object3dCommon::GetInstance());
	object3d_->SetModel(textureFilePath);
	object3d_->SetEnvironmentMapTextureFilePath("resources/human/white.png");
	object3d_->SetTransform(transform);
	transform_ = transform;
}

void Skydome::Update()
{
	// 更新処理
	object3d_->Update();
}

void Skydome::Draw()
{
	// 描画処理
	object3d_->Draw();
}
