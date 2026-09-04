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
	// 登録された敵を更新
	for (const std::unique_ptr<BaseEnemy>& enemy : enemies_)
	{
		// コライダーが参照しているEnemyを無効にしないため、死亡後も実体は保持する
		if (!enemy->IsDead())
		{
			enemy->Update();
		}
	}
}

void EnemyManager::Draw()
{
	// 登録された敵を描画
	for (const std::unique_ptr<BaseEnemy>& enemy : enemies_)
	{
		// 死亡したEnemyは実体だけ保持し、描画は行わない
		if (!enemy->IsDead())
		{
			enemy->Draw();
		}
	}
}

void EnemyManager::Finalize()
{
	// 登録された敵を終了
	enemies_.clear();

	// インスタンスを解放
	instance.reset();
}
