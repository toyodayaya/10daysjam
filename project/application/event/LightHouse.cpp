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
	if (isHit_)
	{
		// ヒットフラグが立っている時の処理

		if (t <= 1.0f)
		{			
			// tを加算
			t += 0.01f;

			// 明るさを線形補間で減少させる
			intencity = Flerp(intencity, 0.0f, t);
		}
		else
		{
			// フラグを戻す
			isHit_ = false;
			t = 0.0f;
			intencity = 0.0f;
		}
	}


#ifdef USE_IMGUI
	ImGui::Begin("LightHouse");
	ImGui::DragFloat3("pos", &transform_.translate.x);
	ImGui::DragFloat("intencity", &intencity);
	ImGui::Text("HP:%d", hp_);

	ImGui::End();
#endif // USE_IMGUI

#ifdef _DEBUG
	// デバッグ描画の更新処理
	debugDraw->SetBoxTranslate(transform_.translate);
	debugDraw->UpdateBox();
#endif // _DEBUG

	object3d_->SetTranslate(transform_.translate);
	object3d_->SetPointLightIntencity(intencity);
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
{
	// ヒットフラグが立っていたら処理しない
	if (isHit_)
	{
		return;
	}

	// ぶつかったオブジェクトのポインタを取得
	hitObject_ = hitObject;

	// ぶつかったオブジェクトのタイプを記録
	hitObjectType_ = hitObjectType;

	// どのオブジェクトにぶつかったか判定
	if (hitObjectType_ == "PlayerSpawn")
	{
		// プレイヤーだった場合の処理
	}
	else if (hitObjectType_ == "EnemySpawn")
	{
		// ボスだった場合の処理
		isHit_ = true;
	}
}

void LightHouse::AddHP(const float& hp)
{
	intencity += hp;

	if (intencity <= 0.0f)
	{
		intencity = 0.0f;
		hp_ = 0;
	}

	hp_ = static_cast<uint32_t>(intencity);
}


uint32_t LightHouse::WithdrawHp(uint32_t maxAmount)
{
	const uint32_t withdrawnHp = (std::min)(hp_, maxAmount);
	hp_ -= withdrawnHp;
	intencity = (std::max)(0.0f, intencity - static_cast<float>(withdrawnHp));

	return withdrawnHp;
}

AABB LightHouse::GetCollisionAabb() const
{
	// 灯台の論理座標を中心に、アプリケーション層で使うAABBを作成する
	const Vector3& center = transform_.translate;
	return {
		{
			center.x - kCollisionAabbHalfSize_.x,
			center.y - kCollisionAabbHalfSize_.y,
			center.z - kCollisionAabbHalfSize_.z
		},
		{
			center.x + kCollisionAabbHalfSize_.x,
			center.y + kCollisionAabbHalfSize_.y,
			center.z + kCollisionAabbHalfSize_.z
		}
	};
}

AABB LightHouse::GetInteractionAabb() const
{
	// 本体の当たり判定とは別に、広めのインタラクト範囲を作成する
	const Vector3& center = transform_.translate;
	return {
		{
			center.x - kInteractionAabbHalfSize_.x,
			center.y - kInteractionAabbHalfSize_.y,
			center.z - kInteractionAabbHalfSize_.z
		},
		{
			center.x + kInteractionAabbHalfSize_.x,
			center.y + kInteractionAabbHalfSize_.y,
			center.z + kInteractionAabbHalfSize_.z
		}
	};
}
