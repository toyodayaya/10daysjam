#include "Enemy.h"
#include "Object3dCommon.h"

void Enemy::Initialize(QuaternionTransform transform, const std::string& filePath)
{
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(Object3dCommon::GetInstance());
	object3d_->SetModel(filePath);
	object3d_->SetEnvironmentMapTextureFilePath("resources/human/white.png");
	object3d_->SetTransform(transform);
}

void Enemy::Finalize()
{}

void Enemy::Update()
{
	object3d_->Update();
}

void Enemy::Draw()
{
	object3d_->Draw();
}
