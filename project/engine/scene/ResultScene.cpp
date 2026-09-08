#include "ResultScene.h"
#include "SceneManager.h"
#include "Input.h"
#include "DamageManager.h"
#include "ImguiManager.h"
#include "TextureManager.h"
#include "SpriteCommon.h"

void ResultScene::Initialize()
{
	// ダメージのビットマップフォントを更新
	DamageManager::GetInstance()->BestDamageBitMapFont();

	// スプライトの読み込むと初期化
	backGround_ = std::make_unique<Sprite>();
	TextureManager::GetInstance()->LoadTexture("resources/UI/result.png");
	backGround_->Initialize(SpriteCommon::GetInstance(), "resources/UI/result.png");

	pressSpace_ = std::make_unique<Sprite>();
	TextureManager::GetInstance()->LoadTexture("resources/UI/pressSpace.png");
	pressSpace_->Initialize(SpriteCommon::GetInstance(), "resources/UI/pressSpace.png");
	pressSpace_->SetPosition(Vector2{ 150.0f,450.0f });

	// 音声読み込み
	resultBgm_ = Audio::GetInstance()->SoundLoadFile("resources/sound/BGM/result.mp3");
	dramrollSE_ = Audio::GetInstance()->SoundLoadFile("resources/sound/SE/dramroll.mp3");
	noticeSE_ = Audio::GetInstance()->SoundLoadFile("resources/sound/SE/notice.mp3");

	// 音声再生
	Audio::GetInstance()->SoundPlayWave(Audio::GetInstance()->GetXAudio2().Get(), dramrollSE_,false);
	
}

void ResultScene::Finalize()
{
	Audio::GetInstance()->SoundStopWave(Audio::GetInstance()->GetXAudio2().Get(), resultBgm_);
	Audio::GetInstance()->SoundUnload(&resultBgm_);
	Audio::GetInstance()->SoundStopWave(Audio::GetInstance()->GetXAudio2().Get(), dramrollSE_);
	Audio::GetInstance()->SoundUnload(&dramrollSE_); 
	Audio::GetInstance()->SoundStopWave(Audio::GetInstance()->GetXAudio2().Get(), noticeSE_);
	Audio::GetInstance()->SoundUnload(&noticeSE_);
}

void ResultScene::Update()
{
	backGround_->Update();
	pressSpace_->Update();

	DamageManager::GetInstance()->BestDamageUpdate();

	if (DamageManager::GetInstance()->GetState() == DamageManager::State::kNotice && !isPlay_)
	{
		// 音声再生
		Audio::GetInstance()->SoundStopWave(Audio::GetInstance()->GetXAudio2().Get(), dramrollSE_);
		Audio::GetInstance()->SoundPlayWave(Audio::GetInstance()->GetXAudio2().Get(), resultBgm_,true);
		Audio::GetInstance()->SoundPlayWave(Audio::GetInstance()->GetXAudio2().Get(), noticeSE_,false);
		isPlay_ = true;
	}

	if (DamageManager::GetInstance()->GetState() == DamageManager::State::kNotice)
	{
		if (Input::GetInstance()->TriggerKey(DIK_SPACE))
		{
			SceneManager::GetInstance()->ChangeScene("TitleScene");
		}
	}

#ifdef USE_IMGUI
	ImGui::Begin("ranking");
	

	ImGui::End();
#endif // USE_IMGUI
}

void ResultScene::Draw()
{
	backGround_->Draw();

	if (DamageManager::GetInstance()->GetState() == DamageManager::State::kNotice)
	{
		pressSpace_->Draw();
	}
	
	DamageManager::GetInstance()->BestDamageDraw();
}
