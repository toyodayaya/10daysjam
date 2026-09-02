#pragma once
#include <memory>
#include <vector>
#include <array>
#include "Sprite.h"

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
	std::vector<int> damageRankings_ = { 1234,54321,123 };
	int nowDamage_ = 0;

	// ダメージの桁数上限
	static const int kNumberArray = 6;
	// 数字用のモデル
	std::array<std::array<std::vector<std::unique_ptr<Sprite>>, kNumberArray>,3> numbers_;
	// 数字記録用の変数
	std::array<std::array<int, kNumberArray>, 3> bitmapNumber_;

public:
	// インスタンス
	static DamageManager* GetInstance();

	// 初期化
	void Initialize();
	// 更新
	void Update();
	// 描画
	void Draw();
	// 終了
	void Finalize();

	// 算出されたダメージをランキングに組み込む関数
	void RankingUpdate(const int& damage);

	// ビットマップフォントを設定
	void RankingBitMapFont();

	// getter
	std::vector<int> GetRanking() { return damageRankings_; }

};

