#include "GamePlayScene.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "StageManager.h"
#include "StageData.h"
#include "SkyboxCommon.h"
#include "Input.h"
#include "SceneManager.h"
#include "DamageManager.h"

void GamePlayScene::Initialize()
{
	// スプライトを読み込む
	TextureManager::GetInstance()->LoadTexture("resources/human/white.png");
	ModelManager::GetInstance()->LoadModel("resources/skydome", "skydome.obj", Model::AnimationType::kNone);

	// objファイルからモデルを読み込む
	ModelManager::GetInstance()->LoadModel("resources/player", "player.obj", Model::AnimationType::kNone);
	ModelManager::GetInstance()->LoadModel("resources/enemy", "enemy.obj", Model::AnimationType::kNone);
	ModelManager::GetInstance()->LoadModel("resources/cube", "cube.obj", Model::AnimationType::kNone);

	// ステージを読み込む
	StageManager::GetInstance()->LoadJsonData("resources/stages", "1.json");
	// ステージを設定する
	stageData_ = StageManager::GetInstance()->FindJsonData("1.json");
	// ステージを作成する
	stageData_->CreateStage("1.json");

	// Skyboxの初期化
	skydomeTransform.translate = { 0.0f,0.0f,0.0f };
	skydomeTransform.scale = { 1.0f,1.0f,1.0f };
	skydomeTransform.rotate = { 0.0f,0.0f,0.0f,1.0f };
	skydome = std::make_unique<Skydome>();
	skydome->Initialize(skydomeTransform, "skydome.obj");
}

void GamePlayScene::Finalize()
{
	stageData_->ClearStage();
	stageData_ = nullptr;
}

void GamePlayScene::Update()
{
	// ステージを更新
	stageData_->Update();

	// 全ての当たり判定を走査
	stageData_->CheckAllCollision();

	// skydomeの更新処理
	skydome->Update();

	if (Input::GetInstance()->TriggerKey(DIK_E))
	{
		SceneManager::GetInstance()->ChangeScene("TitleScene");
	}
}

void GamePlayScene::Draw()
{
	// ステージを描画
	stageData_->Draw();

	// skydomeの描画
	skydome->Draw();
}
