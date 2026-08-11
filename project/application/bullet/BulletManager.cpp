#include "BulletManager.h"
#include <cassert>

std::unique_ptr<BulletManager> BulletManager::instance = nullptr;


BulletManager* BulletManager::GetInstance()
{
	if (instance == nullptr)
	{
		instance = std::make_unique<BulletManager>(ConstructorKey());
	}
	return instance.get();
}

void BulletManager::Update()
{
	// デスフラグが立った弾を削除
	bullets_.erase(std::remove_if(bullets_.begin(), bullets_.end(), []
	(const std::unique_ptr<BaseBullet>& bullet)
		{
			return bullet->IsDead();
		}
	),
		bullets_.end()
	);


	// 登録された敵を更新
	for (const std::unique_ptr<BaseBullet>& bullet : bullets_)
	{
		bullet->Update();
	}
}

void BulletManager::Draw()
{
	// 登録された敵を描画
	for (const std::unique_ptr<BaseBullet>& bullet : bullets_)
	{
		bullet->Draw();
	}
}

void BulletManager::Finalize()
{
	// 登録された敵を終了
	for (const std::unique_ptr<BaseBullet>& bullet : bullets_)
	{
		bullet->Finalize();
	}

	// インスタンスを解放
	instance.reset();
}
