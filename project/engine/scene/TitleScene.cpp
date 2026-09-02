#include "TitleScene.h"
#include "SceneManager.h"
#include "Input.h"
#include "DamageManager.h"
#include "ImguiManager.h"

void TitleScene::Initialize()
{
	ranking_ = DamageManager::GetInstance()->GetRanking();
	DamageManager::GetInstance()->RankingBitMapFont();
}

void TitleScene::Finalize()
{
	
}

void TitleScene::Update()
{
	if (Input::GetInstance()->TriggerKey(DIK_SPACE))
	{
		SceneManager::GetInstance()->ChangeScene("GamePlayScene");
	}

	DamageManager::GetInstance()->Update();

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
	DamageManager::GetInstance()->Draw();
}
