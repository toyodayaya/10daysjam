#include "TitleScene.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "StageManager.h"
#include "StageData.h"
#include "SkyboxCommon.h"

void TitleScene::Initialize()
{
	// スプライトを読み込む
	TextureManager::GetInstance()->LoadTexture("resources/human/white.png");

	// objファイルからモデルを読み込む
	ModelManager::GetInstance()->LoadModel("resources/player", "player.obj", Model::AnimationType::kNone);
	ModelManager::GetInstance()->LoadModel("resources/enemy", "enemy.obj", Model::AnimationType::kNone);
	// テクスチャの読み込み
	TextureManager::GetInstance()->LoadTexture("resources/model/rostock_laage_airport_4k.dds");
	
	// ステージを読み込む
	StageManager::GetInstance()->LoadJsonData("resources/stages", "1.json");
	StageManager::GetInstance()->LoadJsonData("resources/stages", "2.json");
	// ステージを設定する
	stageData_ = StageManager::GetInstance()->FindJsonData("1.json");
	// ステージを作成する
	stageData_->CreateStage("1.json");

	// Skyboxの初期化
	skybox = std::make_unique<Skybox>();
	skybox->Initialize(SkyboxCommon::GetInstance(), "resources/model/rostock_laage_airport_4k.dds");
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

	// Skyboxの更新処理
	skybox->Update();
}

void TitleScene::Draw()
{
	// ステージを描画
	stageData_->Draw();

	// Skyboxの描画
	skybox->Draw();
}
