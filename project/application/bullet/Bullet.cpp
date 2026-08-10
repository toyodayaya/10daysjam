#include "Bullet.h"
#include "CollisionManager.h"
#include "StageData.h"
#include "Object3dCommon.h"
#ifdef _DEBUG
#include "DebugDraw.h"
#include "DebugDrawCommon.h"
#endif // _DEBUG

void Bullet::Initialize(const QuaternionTransform& transform, const std::string& filePath)
{
	// 3Dオブジェクトを初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(Object3dCommon::GetInstance());
	object3d_->SetModel(filePath);
	object3d_->SetEnvironmentMapTextureFilePath("resources/human/white.png");
	object3d_->SetTransform(transform);
	object3d_->SetIsRailCamera(true);
	object3d_->SetOffset(Vector3{ 0.0f,0.0f,10.0f });
	// コライダーを生成
#ifdef _DEBUG
// デバッグ描画用の箱を初期化、生成
	std::unique_ptr<DebugDraw> debugDraw = std::make_unique<DebugDraw>();
	debugDraw->Initialize(DebugDrawCommon::GetInstance(), "resources/human/white.png", DebugDraw::DrawState::kBox);
	debugDraw->SetBoxScale(transform.scale);
	debugDraw->SetBoxTranslate(transform.translate);

#endif // _DEBUG
	StageData::ColliderSpawnData colliders;
	colliders.center = transform.translate;
	colliders.size = transform.scale;
	colliders.parent = this;
	CollisionManager::GetInstance()->SetColliders(colliders);
}

void Bullet::Finalize()
{}

void Bullet::Update()
{
	object3d_->Update();
}

void Bullet::Draw()
{
	object3d_->Draw();
}

void Bullet::OnCollision()
{}
