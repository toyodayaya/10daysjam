#include "Player.h"
#include "Object3dCommon.h"
#include "ModelManager.h"
#include "Model.h"
#include "TextureManager.h"

void Player::Initialize()
{
	ModelManager::GetInstance()->LoadModel("resources/model/model", "axis.obj",Model::AnimationType::kNone);
	// スプライトを読み込む
	TextureManager::GetInstance()->LoadTexture("resources/human/white.png");
	// オブジェクトの初期化
	object3d = std::make_unique<Object3d>();
	object3d->Initialize(Object3dCommon::GetInstance());
}

void Player::Update()
{
	object3d->Update();
}

void Player::Draw()
{
	object3d->Draw();
}
