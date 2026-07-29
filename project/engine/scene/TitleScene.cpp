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
	ModelManager::GetInstance()->LoadModel("resources/player", "player.obj", Model::AnimationType::kNone);
	ModelManager::GetInstance()->LoadModel("resources/enemy", "enemy.obj", Model::AnimationType::kNone);
	
	// ステージを読み込む
	StageManager::GetInstance()->LoadJsonData("resources/stages", "1.json");
	StageManager::GetInstance()->LoadJsonData("resources/stages", "2.json");
	// ステージを設定する
	stageData_ = StageManager::GetInstance()->FindJsonData("1.json");
	// ステージを作成する
	stageData_->CreateStage("1.json");
}

void TitleScene::Finalize()
{
	
}

void TitleScene::Update()
{
	// ステージを更新
	stageData_->Update();

	// 全ての当たり判定を走査
	stageData_->CheckAllCollision();
}

void TitleScene::Draw()
{
	// ステージを描画
	stageData_->Draw();
}
