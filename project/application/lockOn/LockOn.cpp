#include "LockOn.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
#include "EnemyManager.h"

void LockOn::Initialize()
{
	// 3Dレティクルスプライトの初期化
	TextureManager::GetInstance()->LoadTexture("resources/sprite/circle.png");
	reticleSprite_ = std::make_unique<Sprite>();
	reticleSprite_->Initialize(SpriteCommon::GetInstance(), spriteFilePath_);
	reticleSprite_->SetAnchorPoint(Vector2{ 0.5f,0.5f });
	reticleSprite_->SetSize(Vector2{ 64.0f,64.0f });
}

void LockOn::Update()
{
	reticleSprite_->Update();
}

void LockOn::Draw()
{
	// ロックオン時にのみ表示
	if (target_)
	{
		reticleSprite_->Draw();
	}
}

void LockOn::LockOnTarget(const std::unique_ptr<Object3d>& reticle)
{
	// レティクルの座標をスクリーン座標に変換
	Vector3 translate = Project(Vector3{ 0.0f,0.0f,0.0f }, 0.0f, 0.0f, 1280.0f, 720.0f, reticle->GetWorldViewProjection());
	spriteTranslate_ = { translate.x,translate.y };

	// レティクルのビュー座標を取得
	Vector3 viewReticle = Transform(reticle->GetWorldTranslate(), reticle->GetViewMatrix());

	// 対象リストをクリア
	targets.clear();

	// ロックオン判定処理
	for (auto& enemy : EnemyManager::GetInstance()->GetEnemies())
	{
		// ビュー座標系に変換する
		Vector3 viewTranslate = Transform(enemy->GetObject3d()->GetWorldTranslate(), enemy->GetObject3d()->GetViewMatrix());

		if (viewTranslate.z <= viewReticle.z)
		{
			// 自機より手前だったらスキップ
			continue;
		}

		// 敵の座標をスクリーン座標に変換
		Vector3 enemyPos = Project(Vector3{ 0.0f,0.0f,0.0f }, 0.0f, 0.0f, 1280.0f, 720.0f, enemy->GetObject3d()->GetWorldViewProjection());

		
		// Vector2に格納
		Vector2 screenPos = { enemyPos.x,enemyPos.y };
		// スプライトの中心からの距離を判定
		float distance = Distance(spriteTranslate_, screenPos);
		// 距離が規定範囲内なら
		if (distance <= kDistanceLockOn)
		{
			// ロックオン対象リストに追加
			targets.emplace_back(std::make_pair(distance, enemy.get()));
		}
	}

	// 一旦ロックオンを解除
	target_ = nullptr;

	// 対象を絞り込んで座標設定する
	if (!targets.empty())
	{
		// 距離で昇順にソート
		targets.sort();

		// 距離が1番小さい敵をロックオン対象にする
		target_ = targets.front().second;

		// 敵の座標をスクリーン座標に変換
		Vector3 enemyPos = Project(Vector3{ 0.0f,0.0f,0.0f }, 0.0f, 0.0f, 1280.0f, 720.0f, target_->GetObject3d()->GetWorldViewProjection());
		// Vector2に格納
		spriteTranslate_ = { enemyPos.x,enemyPos.y };
		
	}

	// レティクルの座標を設定
	reticleSprite_->SetPosition(spriteTranslate_);
}
