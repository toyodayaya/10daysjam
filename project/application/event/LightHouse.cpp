#include "LightHouse.h"

#ifdef _DEBUG
#include "DebugDrawCommon.h"
#endif // _DEBUG

#include "MathManager.h"
using namespace MathManager;

void LightHouse::Initialize(const QuaternionTransform& transform, const std::string& filePath)
{
	// オブジェクトの初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(Object3dCommon::GetInstance());
	object3d_->SetModel(filePath);
	object3d_->SetEnvironmentMapTextureFilePath("resources/human/white.png");
	object3d_->SetTransform(transform);
	object3d_->SetOffset(Vector3{ 0.0f,0.0f,10.0f });
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
	object3d_->Update();

#ifdef _DEBUG
	// デバッグ描画の更新処理
	debugDraw->UpdateBox();
#endif // _DEBUG
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
