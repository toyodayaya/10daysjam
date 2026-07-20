#include "Enemy.h"
#include "Object3dCommon.h"
#include "ModelManager.h"
#include "Model.h"
#include "TextureManager.h"

void Enemy::Initialize()
{
	// オブジェクトの初期化
	object3d = std::make_unique<Object3d>();
	object3d->Initialize(Object3dCommon::GetInstance());
}

void Enemy::Update()
{
	object3d->Update();
}

void Enemy::Draw()
{
	object3d->Draw();
}
