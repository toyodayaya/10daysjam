#pragma once
#include "BaseEnemy.h"
#include <memory>
#include <vector>

class EnemyManager
{
public:
	// コンストラクタ
	EnemyManager() = default;
	// デストラクタ
	~EnemyManager() = default;
	// コピーコンストラクタとコピー代入演算子を削除
	EnemyManager(const EnemyManager&) = delete;
	EnemyManager& operator=(const EnemyManager&) = delete;
	// インスタンス
	static std::unique_ptr<EnemyManager> instance;

private:
	// 登録済みの敵を記録する配列
	std::vector<std::unique_ptr<BaseEnemy>> enemies_;

public:
	// 更新
	void Update();
	// 描画
	void Draw();
	// 終了
	void Finalize();


	// setter
	void SetEnemies(std::unique_ptr<BaseEnemy> enemy) { enemies_.push_back(std::move(enemy)); }

	// インスタンス
	static EnemyManager* GetInstance();
};

