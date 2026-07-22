#include "TitleScene.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "StageManager.h"
#include "StageData.h"


void TitleScene::Initialize()
{
	// スプライトを読み込む
	TextureManager::GetInstance()->LoadTexture("resources/human/white.png");

	// objファイルからモデルを読み込む
	ModelManager::GetInstance()->LoadModel("resources/model", "axis.obj", Model::AnimationType::kNone);
	ModelManager::GetInstance()->LoadModel("resources/model", "terrain.obj", Model::AnimationType::kNone);
	ModelManager::GetInstance()->LoadModel("resources/model", "fence.obj", Model::AnimationType::kNone);
	ModelManager::GetInstance()->LoadModel("resources/player", "player.obj", Model::AnimationType::kNone);
	
	// ステージを読み込む
	StageManager::GetInstance()->LoadJsonData("resources/stages", "1.json");
	// ステージを設定する
	stageData = StageManager::GetInstance()->FindJsonData("1.json");
	// ステージを作成する
	stageData->CreateStage("1.json");
}

void TitleScene::Finalize()
{
	
}

void TitleScene::Update()
{
	// ステージを更新
	stageData->Update();
}

void TitleScene::Draw()
{
	// ステージを描画
	stageData->Draw();
}
