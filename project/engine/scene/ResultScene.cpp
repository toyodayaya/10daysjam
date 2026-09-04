#include "ResultScene.h"
#include "SceneManager.h"
#include "Input.h"
#include "DamageManager.h"
#include "ImguiManager.h"

void ResultScene::Initialize()
{
	DamageManager::GetInstance()->BestDamageBitMapFont();
}

void ResultScene::Finalize()
{

}

void ResultScene::Update()
{
	DamageManager::GetInstance()->BestDamageUpdate();

	if (DamageManager::GetInstance()->GetState() == DamageManager::State::Notice)
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
	DamageManager::GetInstance()->BestDamageDraw();
}
