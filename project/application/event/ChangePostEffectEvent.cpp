#include "ChangePostEffectEvent.h"
#include "RenderTexture.h"
#include "Input.h"
#ifdef _DEBUG
#include "DebugDrawCommon.h"
#endif // _DEBUG

#include "MathManager.h"
using namespace MathManager;

void ChangePostEffectEvent::Initialize(const QuaternionTransform& transform)
{
#ifdef _DEBUG
	debugDraw = std::make_unique<DebugDraw>();
	debugDraw->Initialize(DebugDrawCommon::GetInstance(), "resources/human/white.png", DebugDraw::DrawState::kBox);
	debugDraw->SetBoxScale(transform.scale);
	debugDraw->SetBoxRotate(transform.rotate);
	debugDraw->SetBoxTranslate(transform.translate);
#endif // _DEBUG
}

void ChangePostEffectEvent::Finalize()
{
#ifdef _DEBUG
	debugDraw.reset();
#endif // _DEBUG
}

void ChangePostEffectEvent::Update()
{
#ifdef _DEBUG
	// デバッグ描画の更新処理
	debugDraw->UpdateBox();
#endif // _DEBUG

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

void ChangePostEffectEvent::Draw()
{
#ifdef _DEBUG
	debugDraw->DrawBox();
#endif // _DEBUG
}

void ChangePostEffectEvent::OnCollision(std::string hitObjectType, BaseCharacter* hitObject)
{}
