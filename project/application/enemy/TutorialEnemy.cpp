#include "TutorialEnemy.h"
#include "Object3dCommon.h"
#include <algorithm>
#include "SceneManager.h"

void TutorialEnemy::Initialize(const QuaternionTransform& transform, const std::string& filePath)
{
	// 3Dオブジェクトを初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(Object3dCommon::GetInstance());
	object3d_->SetModel(filePath);
	object3d_->SetEnvironmentMapTextureFilePath("resources/human/white.png");
	object3d_->SetTransform(transform);
	transform_ = transform;
	hp_ = kInitialHp_;
	isDead_ = false;
}

void TutorialEnemy::Finalize()
{
	
}

void TutorialEnemy::Update()
{
	object3d_->Update();
}

void TutorialEnemy::Draw()
{
	object3d_->Draw();
}

void TutorialEnemy::OnCollision(std::string hitObjectType, BaseCharacter* hitObject)
{
	
}

void TutorialEnemy::AddHP(const float& hp)
{}

void TutorialEnemy::SetMaxHP(const float& hp)
{}

void TutorialEnemy::TakeDamage(int damage)
{
	// 無効なダメージや死亡後の重複ダメージは処理しない
	if (damage <= 0 || isDead_)
	{
		return;
	}

	hp_ = (std::max)(0, hp_ - damage);
	if (hp_ == 0)
	{
		isDead_ = true;
	}
	else
	{
		// 一発で倒しきれなかった場合は回復
		hp_ = kInitialHp_;
	}
}

AABB TutorialEnemy::GetDamageAabb() const
{
	// 爆発判定専用のAABBを作成する
	const Vector3& center = transform_.translate;
	return {
		{
			center.x - kDamageAabbHalfSize_.x,
			center.y - kDamageAabbHalfSize_.y,
			center.z - kDamageAabbHalfSize_.z
		},
		{
			center.x + kDamageAabbHalfSize_.x,
			center.y + kDamageAabbHalfSize_.y,
			center.z + kDamageAabbHalfSize_.z
		}
	};
}
