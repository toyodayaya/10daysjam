#include "ResultScene.h"
#include "SceneManager.h"
#include "Input.h"
#include "DamageManager.h"
#include "ImguiManager.h"
#include "TextureManager.h"
#include "SpriteCommon.h"

void ResultScene::Initialize()
{
	DamageManager::GetInstance()->BestDamageBitMapFont();

	backGround_ = std::make_unique<Sprite>();
	TextureManager::GetInstance()->LoadTexture("resources/UI/result.png");
	backGround_->Initialize(SpriteCommon::GetInstance(), "resources/UI/result.png");

	pressSpace_ = std::make_unique<Sprite>();
	TextureManager::GetInstance()->LoadTexture("resources/UI/pressSpace.png");
	pressSpace_->Initialize(SpriteCommon::GetInstance(), "resources/UI/pressSpace.png");
	pressSpace_->SetPosition(Vector2{ 150.0f,450.0f });
}

void ResultScene::Finalize()
{

}

void ResultScene::Update()
{
	backGround_->Update();
	pressSpace_->Update();

	DamageManager::GetInstance()->BestDamageUpdate();

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
