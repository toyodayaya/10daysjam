#pragma once
#include <memory>
#include <vector>

class DamageManager
{
public:
	// コンストラクタに渡すための鍵
	class ConstructorKey
	{
	private:
		ConstructorKey() = default;
		friend class DamageManager;
	};

	// PassKeyを受け取るコンストラクタ
	explicit DamageManager(ConstructorKey) {}


private:
	// デストラクタ
	~DamageManager() = default;
	// コピーコンストラクタとコピー代入演算子を削除
	DamageManager(const DamageManager&) = delete;
	DamageManager& operator=(const DamageManager&) = delete;
	// インスタンス
	friend std::default_delete<DamageManager>;
	static std::unique_ptr<DamageManager> instance;

	// ダメージ記録用の変数
	std::vector<int> damageRankings_ = { 0,0,0 };
	int nowDamage_ = 0;

public:
	// インスタンス
	static DamageManager* GetInstance();

	// 算出されたダメージをランキングに組み込む関数
	void RankingUpdate(const int& damage);

	// getter
	std::vector<int> GetRanking() { return damageRankings_; }

};

