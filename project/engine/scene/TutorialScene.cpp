#include "TutorialScene.h"
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

void TutorialScene::Initialize()
{
	// スプライトを読み込む
	TextureManager::GetInstance()->LoadTexture("resources/human/white.png");
	TextureManager::GetInstance()->LoadTexture("resources/UI/lighting.png");
	TextureManager::GetInstance()->LoadTexture("resources/UI/respawn.png");
	TextureManager::GetInstance()->LoadTexture("resources/UI/defeat.png");
	TextureManager::GetInstance()->LoadTexture("resources/UI/nextScene.png");
	TextureManager::GetInstance()->LoadTexture("resources/UI/return.png");

	TextureManager::GetInstance()->LoadTexture("resources/UI/space.png");

	// objファイルからモデルを読み込む
	ModelManager::GetInstance()->LoadModel("resources/player", "player.obj", Model::AnimationType::kNone);
	ModelManager::GetInstance()->LoadModel("resources/enemy", "enemy.obj", Model::AnimationType::kNone);
	ModelManager::GetInstance()->LoadModel("resources/cube", "cube.obj", Model::AnimationType::kNone);
	ModelManager::GetInstance()->LoadModel("resources/skydome", "skydome.obj", Model::AnimationType::kNone);

	// ステージを読み込む
	StageManager::GetInstance()->LoadJsonData("resources/stages", "tutorial.json");
	// ステージを設定する
	stageData_ = StageManager::GetInstance()->FindJsonData("tutorial.json");
	// ステージを作成する
	stageData_->CreateStage("tutorial.json");

	// Skyboxの初期化
	skydomeTransform.translate = { 0.0f,0.0f,0.0f };
	skydomeTransform.scale = { 1.0f,1.0f,1.0f };
	skydomeTransform.rotate = { 0.0f,0.0f,0.0f,1.0f };
	skydome = std::make_unique<Skydome>();
	skydome->Initialize(skydomeTransform, "skydome.obj");

	// スプライトの初期化
	lighting_ = std::make_unique<Sprite>();
	lighting_->Initialize(SpriteCommon::GetInstance(), "resources/UI/lighting.png");
	lighting_->SetPosition(Vector2{ 0.0f,500.0f });

	respawn_ = std::make_unique<Sprite>();
	respawn_->Initialize(SpriteCommon::GetInstance(), "resources/UI/respawn.png");
	respawn_->SetPosition(Vector2{ 0.0f,500.0f });

	defeat_ = std::make_unique<Sprite>();
	defeat_->Initialize(SpriteCommon::GetInstance(), "resources/UI/defeat.png");
	defeat_->SetPosition(Vector2{ 0.0f,500.0f });

	next_ = std::make_unique<Sprite>();
	next_->Initialize(SpriteCommon::GetInstance(), "resources/UI/nextScene.png");
	next_->SetPosition(Vector2{ 0.0f,500.0f });

	goTitle_ = std::make_unique<Sprite>();
	goTitle_->Initialize(SpriteCommon::GetInstance(), "resources/UI/return.png");
	goTitle_->SetPosition(Vector2{ 0.0f,400.0f });

	// 音声読み込み
	tutorialBgm_ = Audio::GetInstance()->SoundLoadFile("resources/sound/BGM/tutorial.mp3");
	// 音声再生
	Audio::GetInstance()->SoundPlayWave(Audio::GetInstance()->GetXAudio2().Get(), tutorialBgm_);
}

void TutorialScene::Finalize()
{
	stageData_->ClearStage();
	stageData_ = nullptr;
	Audio::GetInstance()->SoundStopWave(Audio::GetInstance()->GetXAudio2().Get(), tutorialBgm_);
	Audio::GetInstance()->SoundUnload(&tutorialBgm_);
}

void TutorialScene::Update()
{
	// ステージを更新
	stageData_->Update();

	// 全ての当たり判定を走査
	stageData_->CheckAllCollision();

	// Rキーが押されたらタイトルへ
	if (Input::GetInstance()->TriggerKey(DIK_R))
	{
		SceneManager::GetInstance()->ChangeScene("TitleScene");
	}

	switch (phase_)
	{
	case kLighting:
	{
		// スプライトの更新
		lighting_->Update();

		// 最も明るい灯台のHPを取得
		LightHouse* respawnLightHouse = EventManager::GetInstance()->GetHighestHpLightHouse();
		if (respawnLightHouse != nullptr)
		{
			if (respawnLightHouse->GetHp() >= 15)
			{
				// 次のフェーズへ
				phase_ = kRespawn;
				return;
			}
		}

		
		break;
	}

	case kRespawn:
	{
		// スプライトの更新
		respawn_->Update();

		if (Input::GetInstance()->TriggerKey(DIK_SPACE))
		{
			// プレイヤーが自爆したら次のフェーズへ
			phase_ = kDefeat;
			return;
		}

		
		break;
	}

	case kDefeat:
	{
		// スプライトの更新
		defeat_->Update();

		// 敵が死亡したら
		if (EnemyManager::GetInstance()->GetIsDeadEnemy())
		{
			// 次のフェーズへ
			phase_ = kNextScene;
			return;
		}

		
		break;
	}

	case kNextScene:
	{
		// スプライトの更新
		next_->Update();

		// エンターキーで次のシーンへ
		if (Input::GetInstance()->TriggerKey(DIK_RETURN))
		{
			SceneManager::GetInstance()->ChangeScene("GamePlayScene");
		}


	}
	}

	// スプライトの更新
	goTitle_->Update();

	// skydomeの更新処理
	skydome->Update();
}

void TutorialScene::Draw()
{
	// ステージを描画
	stageData_->Draw();

	// skydomeの描画
	skydome->Draw();

	// スプライトの描画
	switch (phase_)
	{
	case kLighting:
		lighting_->Draw();
		break;

	case kRespawn:
		respawn_->Draw();
		break;

	case kDefeat:
		defeat_->Draw();
		break;

	case kNextScene:
		next_->Draw();
		break;

	}

	goTitle_->Draw();
}
