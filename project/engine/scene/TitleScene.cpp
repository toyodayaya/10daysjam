#include "TitleScene.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "StageManager.h"
#include "StageData.h"
#include "Object3dCommon.h"

void TitleScene::Initialize()
{
	// スプライトを読み込む
	TextureManager::GetInstance()->LoadTexture("resources/human/white.png");

	// objファイルからモデルを読み込む
	ModelManager::GetInstance()->LoadModel("resources/model", "axis.obj", Model::AnimationType::kNone);
	ModelManager::GetInstance()->LoadModel("resources/model", "terrain.obj", Model::AnimationType::kNone);
	ModelManager::GetInstance()->LoadModel("resources/model", "fence.obj", Model::AnimationType::kNone);
	ModelManager::GetInstance()->LoadModel("resources/player", "player.obj", Model::AnimationType::kNone);
	ModelManager::GetInstance()->LoadModel("resources/enemy", "enemy.obj", Model::AnimationType::kNone);
	ModelManager::GetInstance()->LoadModel("resources/human", "walk.gltf", Model::AnimationType::kMove);
	
	// ステージを読み込む
	StageManager::GetInstance()->LoadJsonData("resources/stages", "1.json");
	// ステージを設定する
	stageData_ = StageManager::GetInstance()->FindJsonData("1.json");
	// ステージを作成する
	stageData_->CreateStage("1.json");

	// 3Dオブジェクトの初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(Object3dCommon::GetInstance());
	object3d_->SetModel("walk.gltf");
	object3d_->SetEnvironmentMapTextureFilePath("resources/human/white.png");
}

void TitleScene::Finalize()
{
	
}

void TitleScene::Update()
{
	// アニメーションモデルを更新
	object3d_->Update();

	// ステージを更新
	stageData_->Update();

	// 全ての当たり判定を走査
	stageData_->CheckAllCollision();
}

void TitleScene::Draw()
{
	// アニメーションモデルを描画
	object3d_->Draw();

	// ステージを描画
	stageData_->Draw();

}
