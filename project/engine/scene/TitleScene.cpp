#include "TitleScene.h"
#include "SceneManager.h"
#include "Input.h"
#include "DamageManager.h"
#include "ImguiManager.h"
#include "TextureManager.h"
#include "SpriteCommon.h"

void TitleScene::Initialize()
{
	// ランキングの変数を宣言
	ranking_ = DamageManager::GetInstance()->GetRanking();
	DamageManager::GetInstance()->RankingBitMapFont();

	// テクスチャデータを読み込む
	TextureManager::GetInstance()->LoadTexture("resources/UI/title.png");
	TextureManager::GetInstance()->LoadTexture("resources/UI/play.png");
	TextureManager::GetInstance()->LoadTexture("resources/UI/tutorial.png");
	TextureManager::GetInstance()->LoadTexture("resources/UI/cursor.png");

	// スプライトの生成と初期化
	backGround_ = std::make_unique<Sprite>();
	backGround_->Initialize(SpriteCommon::GetInstance(), "resources/UI/title.png");

	play_ = std::make_unique<Sprite>();
	play_->Initialize(SpriteCommon::GetInstance(), "resources/UI/play.png");
	play_->SetPosition(Vector2{ 600.0f,400.0f });

	tutorial_ = std::make_unique<Sprite>();
	tutorial_->Initialize(SpriteCommon::GetInstance(), "resources/UI/tutorial.png");
	tutorial_->SetPosition(Vector2{ 600.0f,500.0f });

	cursor_ = std::make_unique<Sprite>();
	cursor_->Initialize(SpriteCommon::GetInstance(), "resources/UI/cursor.png");
	cursor_->SetPosition(Vector2{ 600.0f,500.0f });

	// 音声読み込み
	titleBgm_ = Audio::GetInstance()->SoundLoadFile("resources/sound/BGM/title.mp3");
	selectSE_ = Audio::GetInstance()->SoundLoadFile("resources/sound/SE/select.wav");
	enterSE_ = Audio::GetInstance()->SoundLoadFile("resources/sound/SE/enter.wav");
	// 音声再生
	Audio::GetInstance()->SoundPlayWave(Audio::GetInstance()->GetXAudio2().Get(), titleBgm_);
}

void TitleScene::Finalize()
{
	Audio::GetInstance()->SoundStopWave(Audio::GetInstance()->GetXAudio2().Get(), titleBgm_);
	Audio::GetInstance()->SoundUnload(&titleBgm_);
	Audio::GetInstance()->SoundStopWave(Audio::GetInstance()->GetXAudio2().Get(), selectSE_);
	Audio::GetInstance()->SoundUnload(&selectSE_); 
	Audio::GetInstance()->SoundStopWave(Audio::GetInstance()->GetXAudio2().Get(), enterSE_);
	Audio::GetInstance()->SoundUnload(&enterSE_);
}

void TitleScene::Update()
{
	switch (nextScene_)
	{
	case kTutorial:
	{
		if (Input::GetInstance()->TriggerKey(DIK_SPACE))
		{
			// 音声再生
			Audio::GetInstance()->SoundPlayWave(Audio::GetInstance()->GetXAudio2().Get(), enterSE_);
			// スペースキーでチュートリアルへ
			SceneManager::GetInstance()->ChangeScene("TutorialScene");
			break;
		}


		if (Input::GetInstance()->TriggerKey(DIK_UPARROW) || Input::GetInstance()->TriggerKey(DIK_DOWNARROW) ||
			Input::GetInstance()->TriggerKey(DIK_S) || Input::GetInstance()->TriggerKey(DIK_W))
		{
			// カーソルを動かす
			Vector2 pos = cursor_->GetPosition();
			pos.y = play_->GetPosition().y;
			cursor_->SetPosition(pos);
			nextScene_ = kPlay;
			// 音声再生
			Audio::GetInstance()->SoundPlayWave(Audio::GetInstance()->GetXAudio2().Get(), selectSE_);
		}

		break;
	}

	case kPlay:
	{
		if (Input::GetInstance()->TriggerKey(DIK_SPACE))
		{
			// 音声再生
			Audio::GetInstance()->SoundPlayWave(Audio::GetInstance()->GetXAudio2().Get(), enterSE_);
			// スペースキーでゲームシーンへ
			SceneManager::GetInstance()->ChangeScene("GamePlayScene");
			break;
		}

		if (Input::GetInstance()->TriggerKey(DIK_UPARROW) || Input::GetInstance()->TriggerKey(DIK_DOWNARROW) ||
			Input::GetInstance()->TriggerKey(DIK_S) || Input::GetInstance()->TriggerKey(DIK_W))
		{
			// カーソルを動かす
			Vector2 pos = cursor_->GetPosition();
			pos.y = tutorial_->GetPosition().y;
			cursor_->SetPosition(pos);
			nextScene_ = kTutorial;
			// 音声再生
			Audio::GetInstance()->SoundPlayWave(Audio::GetInstance()->GetXAudio2().Get(), selectSE_);
		}

		break;
	}
	}

	DamageManager::GetInstance()->Update();
	backGround_->Update();
	play_->Update();
	tutorial_->Update();
	cursor_->Update();

#ifdef USE_IMGUI
	ImGui::Begin("ranking");
	ImGui::Text("1st:%d", ranking_[0]);
	ImGui::Text("2nd:%d", ranking_[1]);
	ImGui::Text("3rd:%d", ranking_[2]);

	ImGui::End();
#endif // USE_IMGUI
}

void TitleScene::Draw()
{
	backGround_->Draw();
	play_->Draw();
	tutorial_->Draw();
	cursor_->Draw();
	DamageManager::GetInstance()->Draw();
}
