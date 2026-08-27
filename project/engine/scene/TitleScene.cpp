#include "TitleScene.h"
#include "SceneManager.h"
#include "Input.h"


void TitleScene::Initialize()
{
	
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
}

void TitleScene::Draw()
{
	
}
