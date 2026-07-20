#include "TitleScene.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "StageManager.h"
#include "StageData.h"
#include "RenderTexture.h"
#include "Input.h"

void TitleScene::Initialize()
{
	// スプライトを読み込む
	TextureManager::GetInstance()->LoadTexture("resources/human/white.png");

	// objファイルからモデルを読み込む
	ModelManager::GetInstance()->LoadModel("resources/model", "axis.obj", Model::AnimationType::kNone);
	ModelManager::GetInstance()->LoadModel("resources/model", "terrain.obj", Model::AnimationType::kNone);
	ModelManager::GetInstance()->LoadModel("resources/model", "fence.obj", Model::AnimationType::kNone);
	
	// ステージを読み込む
	StageManager::GetInstance()->LoadJsonData("resources/stages", "1.json");
	// ステージを設定する
	stageData = StageManager::GetInstance()->FindJsonData("1.json");
	// ステージを作成する
	stageData->CreateStage("1.json");
}

void TitleScene::Finalize()
{
	
}

void TitleScene::Update()
{
	// ステージを更新
	stageData->Update();

	// キー入力でPostEffectを変更
	ChangePostEffect();
}

void TitleScene::Draw()
{
	// ステージを描画
	stageData->Draw();
}

void TitleScene::ChangePostEffect()
{
	if (Input::GetInstance()->TriggerKey(DIK_0))
	{
		// 0を押したら通常へ
		RenderTexture::GetInstance()->SetPostEffect(RenderTexture::PostEffect::kNormal);
	}
	else if (Input::GetInstance()->TriggerKey(DIK_1))
	{
		// 1を押したらGrayScaleへ
		RenderTexture::GetInstance()->SetPostEffect(RenderTexture::PostEffect::kGrayScale);
	}
	else if (Input::GetInstance()->TriggerKey(DIK_2))
	{
		// 2を押したらSepiaScaleへ
		RenderTexture::GetInstance()->SetPostEffect(RenderTexture::PostEffect::kSepiaScale);
	}
	else if (Input::GetInstance()->TriggerKey(DIK_3))
	{
		// 3を押したらVignettingへ
		RenderTexture::GetInstance()->SetPostEffect(RenderTexture::PostEffect::kVignetting);
	}
	else if (Input::GetInstance()->TriggerKey(DIK_4))
	{
		// 4を押したらBoxFilterへ
		RenderTexture::GetInstance()->SetPostEffect(RenderTexture::PostEffect::kBoxFilter);
	}
	else if (Input::GetInstance()->TriggerKey(DIK_5))
	{
		// 5を押したらGaussianFilterへ
		RenderTexture::GetInstance()->SetPostEffect(RenderTexture::PostEffect::kGaussianFilter);
	}
	else if (Input::GetInstance()->TriggerKey(DIK_6))
	{
		// 6を押したらOutlineへ
		RenderTexture::GetInstance()->SetPostEffect(RenderTexture::PostEffect::kOutline);
	}
	else if (Input::GetInstance()->TriggerKey(DIK_7))
	{
		// 7を押したらRadialBlurへ
		RenderTexture::GetInstance()->SetPostEffect(RenderTexture::PostEffect::kRadialBlur);
	}
	else if (Input::GetInstance()->TriggerKey(DIK_8))
	{
		// 8を押したらDissolveへ
		RenderTexture::GetInstance()->SetPostEffect(RenderTexture::PostEffect::kDissolve);
	}
	else if (Input::GetInstance()->TriggerKey(DIK_9))
	{
		// 9を押したらRandomへ
		RenderTexture::GetInstance()->SetPostEffect(RenderTexture::PostEffect::kRandom);
	}
}
