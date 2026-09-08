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

	enum State
	{
		kRoll,
		kNotice
	};


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
	int bestDamage_ = 0;

	// ダメージの桁数上限
	static const int kNumberArray = 6;
	// 数字用のモデル
	std::array<std::array<std::vector<std::unique_ptr<Sprite>>, kNumberArray>,4> numbers_;
	// 数字記録用の変数
	std::array<std::array<int, kNumberArray>, 4> bitmapNumber_;
	// 順位スプライト配列
	std::vector<std::unique_ptr<Sprite>> rankSprite_;

	// 発表フェーズ
	State phase_ = kRoll;
	// ドラムロールタイマー
	float drumRollTimer_ = 2;

public:
	// インスタンス
	static DamageManager* GetInstance();

	// 初期化
	void Initialize();
	// 更新
	void Update();
	void BestDamageUpdate();
	// 描画
	void Draw();
	void BestDamageDraw();
	// 終了
	void Finalize();

	// 算出されたダメージをランキングに組み込む関数
	void RankingUpdate(const int& damage);

	// ビットマップフォントを設定
	void RankingBitMapFont();
	void BestDamageBitMapFont();

	// 1プレイ内の最高ダメージを記録する関数
	void SetOnePlayBestDamage(const int& damage);

	// getter
	std::vector<int> GetRanking() { return damageRankings_; }
	State GetState() { return phase_; }

};

