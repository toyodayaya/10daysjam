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

void TutorialScene::Initialize()
{
	// スプライトを読み込む
	TextureManager::GetInstance()->LoadTexture("resources/human/white.png");
	ModelManager::GetInstance()->LoadModel("resources/skydome", "skydome.obj", Model::AnimationType::kNone);

	// objファイルからモデルを読み込む
	ModelManager::GetInstance()->LoadModel("resources/player", "player.obj", Model::AnimationType::kNone);
	ModelManager::GetInstance()->LoadModel("resources/enemy", "enemy.obj", Model::AnimationType::kNone);
	ModelManager::GetInstance()->LoadModel("resources/cube", "cube.obj", Model::AnimationType::kNone);

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
}

void TutorialScene::Finalize()
{
	stageData_->ClearStage();
	stageData_ = nullptr;
}

void TutorialScene::Update()
{
	// ステージを更新
	stageData_->Update();

	// 全ての当たり判定を走査
	stageData_->CheckAllCollision();

	// skydomeの更新処理
	skydome->Update();

	switch (phase_)
	{
	case kLighting:
	{
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
		// エンターキーで次のシーンへ
		if (Input::GetInstance()->TriggerKey(DIK_RETURN))
		{
			SceneManager::GetInstance()->ChangeScene("GamePlayScene");
		}
	}
	}
}

void TutorialScene::Draw()
{
	// ステージを描画
	stageData_->Draw();

	// skydomeの描画
	skydome->Draw();
}
