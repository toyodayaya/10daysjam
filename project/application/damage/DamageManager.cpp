#include "DamageManager.h"
#include <cassert>

std::unique_ptr<DamageManager> DamageManager::instance = nullptr;


DamageManager* DamageManager::GetInstance()
{
	if (instance == nullptr)
	{
		instance = std::make_unique<DamageManager>(ConstructorKey());
	}
	return instance.get();
}

void DamageManager::RankingUpdate(const int& damage)
{
	// 引数で受け取ってメンバ変数として記録
	nowDamage_ = damage;

	// ランキングを更新
	for (int i = 0; i < damageRankings_.size(); i++)
	{
		// 今のダメージが記録されたダメージより大きい場合
		if (nowDamage_ >= damageRankings_[i])
		{
			// ランキングを入れ替える
			int damage = damageRankings_[i];
			damageRankings_[i] = nowDamage_;
			nowDamage_ = damage;
		}
	}
}
