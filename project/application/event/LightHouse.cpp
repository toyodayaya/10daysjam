#include "LightHouse.h"
#ifdef _DEBUG
#include "DebugDrawCommon.h"
#endif // _DEBUG
#include "ImGuiManager.h"
#include "MathManager.h"
#include <algorithm>
using namespace MathManager;

void LightHouse::Initialize(const QuaternionTransform& transform, const std::string& filePath)
{
	// オブジェクトの初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(Object3dCommon::GetInstance());
	object3d_->SetModel(filePath);
	object3d_->SetEnvironmentMapTextureFilePath("resources/human/white.png");
	object3d_->SetTransform(transform);
	object3d_->SetPointLightPos(transform.translate);
	transform_ = transform;

	isDead_ = false;

#ifdef _DEBUG
	debugDraw = std::make_unique<DebugDraw>();
	debugDraw->Initialize(DebugDrawCommon::GetInstance(), "resources/human/white.png", DebugDraw::DrawState::kBox);
	debugDraw->SetBoxScale(transform.scale);
	debugDraw->SetBoxRotate(transform.rotate);
	debugDraw->SetBoxTranslate(transform.translate);
#endif // _DEBUG
}

void LightHouse::Finalize()
{
#ifdef _DEBUG
	debugDraw.reset();
#endif // _DEBUG
}

void LightHouse::Update()
{


#ifdef _DEBUG
	// デバッグ描画の更新処理
	debugDraw->UpdateBox();
#endif // _DEBUG

#ifdef USE_IMGUI
	ImGui::Begin("LightHouse");
	Vector3 transform = object3d_->GetWorldTranslate();
	ImGui::DragFloat3("pos", &transform.x);
	ImGui::DragFloat("intencity", &intencity);
	ImGui::DragFloat("decay", &decay);

	ImGui::End();

#endif // USE_IMGUI
	object3d_->SetPointLightIntencity(intencity);
	object3d_->SetPointLightDecay(decay);
	object3d_->Update();
}

void LightHouse::Draw()
{
	object3d_->Draw();

#ifdef _DEBUG
	debugDraw->DrawBox();
#endif // _DEBUG
}

void LightHouse::OnCollision(std::string hitObjectType, BaseCharacter* hitObject)
{}

void LightHouse::AddHP(const float& hp)
{
	intencity += hp;
	hp_ += static_cast<uint32_t>(hp);

	if (intencity <= 0.0f)
	{
		intencity = 0.0f;
	}
}

uint32_t LightHouse::WithdrawHp(uint32_t maxAmount)
{
	const uint32_t withdrawnHp = (std::min)(hp_, maxAmount);
	hp_ -= withdrawnHp;
	intencity = (std::max)(0.0f, intencity - static_cast<float>(withdrawnHp));

	return withdrawnHp;
}
