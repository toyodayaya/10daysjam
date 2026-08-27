#include "Bullet.h"
#include "CollisionManager.h"
#include "StageData.h"
#include "Object3dCommon.h"
#ifdef _DEBUG
#include "DebugDraw.h"
#include "DebugDrawCommon.h"

#endif // _DEBUG

void Bullet::Initialize(const QuaternionTransform& transform, const std::string& filePath, const Vector3& velocity, bool isRailCamera)
{
	// 3Dオブジェクトを初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(Object3dCommon::GetInstance());
	object3d_->SetModel(filePath);
	object3d_->SetEnvironmentMapTextureFilePath("resources/human/white.png");
	object3d_->SetTransform(transform);
	object3d_->SetIsRailCamera(isRailCamera);
	object3d_->SetOffset(Vector3(0.0f, 0.0f, 10.0f));
	velocity_ = velocity;
	transform_ = transform;
	// コライダーを生成
#ifdef _DEBUG
// デバッグ描画用の箱を初期化、生成
	debugDraw = std::make_unique<DebugDraw>();
	debugDraw->Initialize(DebugDrawCommon::GetInstance(), "resources/human/white.png", DebugDraw::DrawState::kBox);
	debugDraw->SetBoxScale(transform.scale);
	debugDraw->SetBoxTranslate(transform.translate);
	debugDraw->SetRotate(transform.rotate);
	debugDraw->SetIsRailCamera(isRailCamera);
	debugDraw->SetOffset(Vector3(0.0f, 0.0f, 10.0f));

#endif // _DEBUG
	StageData::ColliderSpawnData colliders;
	colliders.center = Vector3{ 0.0f,0.0f,0.0f };
	colliders.size = transform.scale;
	colliders.parent = this;
	colliders.objectType = "PlayerSpawn";
	CollisionManager::GetInstance()->SetColliders(colliders);

	isDead_ = false;
}

void Bullet::Finalize()
{

}

void Bullet::Update()
{
	// 生存時間を更新
	currentTime_ += kDeltaTime;

	// 生存時間を過ぎたら削除
	if (currentTime_ >= kLifeTime_)
	{
		isDead_ = true;
	}

	// 座標を更新
	transform_.translate = Vector3Add(transform_.translate, velocity_);
	object3d_->SetTranslate(transform_.translate);
	object3d_->Update();

#ifdef _DEBUG
	debugDraw->SetBoxTranslate(transform_.translate);
	debugDraw->UpdateBox();
#endif // _DEBUG

}

void Bullet::Draw()
{
	object3d_->Draw();
#ifdef _DEBUG
	debugDraw->DrawBox();

#endif // _DEBUG


}

void Bullet::OnCollision()
{
	isDead_ = true;
}
