#include "LightHouse.h"
#include "Enemy.h"
#include "TextureManager.h"
#include "SpriteCommon.h"
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

	// スプライトの読み込みと初期化
	TextureManager::GetInstance()->LoadTexture("resources/UI/controller.png");
	TextureManager::GetInstance()->LoadTexture("resources/UI/hp.png");
	TextureManager::GetInstance()->LoadTexture("resources/UI/bar.png");

	controller_ = std::make_unique<Sprite>();
	controller_->Initialize(SpriteCommon::GetInstance(), "resources/UI/controller.png");
	controller_->SetAnchorPoint(Vector2{ 0.5f,0.5f });

	hpSprite_ = std::make_unique<Sprite>();
	hpSprite_->Initialize(SpriteCommon::GetInstance(), "resources/UI/hp.png");
	hpSprite_->SetAnchorPoint(Vector2{ 1.5f,2.0f });

	bar_ = std::make_unique<Sprite>();
	bar_->Initialize(SpriteCommon::GetInstance(), "resources/UI/bar.png");
	bar_->SetAnchorPoint(Vector2{ 0.0f,2.0f });
	bar_->SetSize(Vector2{ 0.0f,50.0f });

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

	ImGui::Text("Stored HP: %u", hp_);


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

	if (isHitPlayer_)
	{
		// オブジェクトの座標をスクリーン座標に変換
		Vector3 translate = Project(object3d_->GetTranslate(), 0.0f, 0.0f, 1280.0f, 720.0f, object3d_->GetViewProjection());
		Vector2 pos = { translate.x,translate.y };
		// スプライトの位置を設定
		controller_->SetPosition(pos);
		hpSprite_->SetPosition(pos);
		pos.x -= 40.0f;
		bar_->SetPosition(pos);
	}

	// スプライトを更新
	controller_->Update();
	hpSprite_->Update();
	bar_->Update();

	isHitPlayer_ = false;
}

void LightHouse::Draw()
{
	object3d_->Draw();

	// 操作案内を表示
	if (isHitPlayer_)
	{
		controller_->Draw();
		hpSprite_->Draw();
		bar_->Draw();
	}

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
		// プレイヤーだった場合
		isHitPlayer_ = true;

	}
	else if (hitObjectType_ == "EnemySpawn")
	{
		// 灯台突進中のボスに当たった場合だけHPを消費する。
		// 叩きつけ・巡回・帰還中の接触では灯台に影響させない。
		Enemy* enemy = dynamic_cast<Enemy*>(hitObject_);
		if (enemy != nullptr && enemy->IsLighthouseAttackContactActive())
		{
			SetIsHit(true);
		}
	}
}

void LightHouse::AddHP(const float& hp)
{
	// ゲーム上の保有HPと表示上の明るさを両方更新する
	if (hp >= 0.0f)
	{
		hp_ += static_cast<uint32_t>(hp);
	}
	else
	{
		const uint32_t decreaseHp = static_cast<uint32_t>(-hp);
		hp_ -= (std::min)(hp_, decreaseHp);
	}

	intencity += hp;

	if (intencity <= 0.0f)
	{
		intencity = 0.0f;
		hp_ = 0;
	}

	hp_ = static_cast<uint32_t>(intencity);

	Vector2 scale = bar_->GetSize();
	scale.x = intencity;
	bar_->SetSize(scale);
	
}

void LightHouse::SetMaxHP(const float& hp)
{}


void LightHouse::SetIsHit(bool isHit)
{
	// リスポーンに使用された、またはEnemyに破壊された灯台の保有HPを消費する
	if (isHit && !isHit_)
	{
		hp_ = 0;
		t = 0.0f;
	}

	isHit_ = isHit;
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
