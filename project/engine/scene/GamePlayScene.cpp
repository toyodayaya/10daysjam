#include "GamePlayScene.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "StageManager.h"
#include "StageData.h"
#include "SkyboxCommon.h"
#include "Input.h"
#include "SceneManager.h"
#include "DamageManager.h"
#include "EnemyManager.h"
#include "SpriteCommon.h"

void GamePlayScene::Initialize()
{
	// スプライトを読み込む
	TextureManager::GetInstance()->LoadTexture("resources/human/white.png");
	TextureManager::GetInstance()->LoadTexture("resources/UI/space.png");
	TextureManager::GetInstance()->LoadTexture("resources/UI/return.png");

	// objファイルからモデルを読み込む
	ModelManager::GetInstance()->LoadModel("resources/player", "player.obj", Model::AnimationType::kNone);
	ModelManager::GetInstance()->LoadModel("resources/enemy", "enemy.obj", Model::AnimationType::kNone);
	ModelManager::GetInstance()->LoadModel("resources/lighthouse", "lighthouse.obj", Model::AnimationType::kNone);
	ModelManager::GetInstance()->LoadModel("resources/skydome", "skydome.obj", Model::AnimationType::kNone);

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

	// スプライトの初期化
	space_ = std::make_unique<Sprite>();
	space_->Initialize(SpriteCommon::GetInstance(), "resources/UI/space.png");
	space_->SetPosition(Vector2{ -20.0f,550.0f });

	goTitle_ = std::make_unique<Sprite>();
	goTitle_->Initialize(SpriteCommon::GetInstance(), "resources/UI/return.png");
	goTitle_->SetPosition(Vector2{ 40.0f,400.0f });

	// 音声読み込み
	playBgm_ = Audio::GetInstance()->SoundLoadFile("resources/sound/BGM/gameplay.mp3");
	// 音声再生
	Audio::GetInstance()->SoundPlayWave(Audio::GetInstance()->GetXAudio2().Get(), playBgm_);
}

void GamePlayScene::Finalize()
{
	stageData_->ClearStage();
	stageData_ = nullptr;
	Audio::GetInstance()->SoundStopWave(Audio::GetInstance()->GetXAudio2().Get(), playBgm_);
	Audio::GetInstance()->SoundUnload(&playBgm_);
}

void GamePlayScene::Update()
{
	
	// ステージを更新
	stageData_->Update();

	// スプライトの更新
	space_->Update();
	goTitle_->Update();

	// 全ての当たり判定を走査
	stageData_->CheckAllCollision();

	// skydomeの更新処理
	skydome->Update();

	// 死亡したら画面遷移
	if (EnemyManager::GetInstance()->GetIsDeadEnemy())
	{
		SceneManager::GetInstance()->ChangeScene("ResultScene");
		return;
	}
	
	if (Input::GetInstance()->TriggerKey(DIK_R))
	{
		// Rキーが押されたらタイトルへ
		SceneManager::GetInstance()->ChangeScene("ResultScene");
	}
}

void GamePlayScene::Draw()
{
	
	// ステージを描画
	stageData_->Draw();

	// スプライトの描画
	space_->Draw();
	goTitle_->Draw();

	// skydomeの描画
	skydome->Draw();
}
