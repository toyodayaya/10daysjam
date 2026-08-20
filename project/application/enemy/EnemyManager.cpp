#include "EnemyManager.h"
#include <cassert>

std::unique_ptr<EnemyManager> EnemyManager::instance = nullptr;


EnemyManager* EnemyManager::GetInstance()
{
	if (instance == nullptr)
	{
		instance = std::make_unique<EnemyManager>(ConstructorKey());
	}
	return instance.get();
}

void EnemyManager::Update()
{
	// デスフラグが立った弾を削除
	enemies_.erase(std::remove_if(enemies_.begin(), enemies_.end(), []
	(const std::unique_ptr<BaseEnemy>& enemy)
		{
			return enemy->IsDead();
		}
	),
		enemies_.end()
	);

	// 登録された敵を更新
	for (const std::unique_ptr<BaseEnemy>& enemy : enemies_)
	{
		enemy->Update();
	}
}

void EnemyManager::Draw()
{
	// 登録された敵を描画
	for (const std::unique_ptr<BaseEnemy>& enemy : enemies_)
	{
		enemy->Draw();
	}
}

void EnemyManager::Finalize()
{
	// 登録された敵を終了
	for (const std::unique_ptr<BaseEnemy>& enemy : enemies_)
	{
		enemy->Finalize();
	}

	// インスタンスを解放
	instance.reset();
}
