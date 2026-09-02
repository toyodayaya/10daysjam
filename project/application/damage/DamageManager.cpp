#include "DamageManager.h"
#include "TextureManager.h"
#include "SpriteCommon.h"
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

void DamageManager::Initialize()
{
	// 数字用の画像を読み込む
	for (size_t r = 0; r < 3; ++r)
	{
		for (size_t i = 0; i < kNumberArray; ++i)
		{
			numbers_[r][i].reserve(10);

			for (size_t j = 0; j < 10; ++j)
			{
				std::string filePath = "resources/numbers/" + std::to_string(j) + ".png";
				TextureManager::GetInstance()->LoadTexture(filePath);
				auto sprite = std::make_unique<Sprite>();
				sprite->Initialize(SpriteCommon::GetInstance(), filePath);
				numbers_[r][i].push_back(std::move(sprite));
			}
		}
	}
}

void DamageManager::Update()
{
	for (size_t r = 0; r < 3; ++r)
	{
		for (size_t i = 0; i < kNumberArray; ++i)
		{
			int number = bitmapNumber_[r][i];
			Vector2 pos = { static_cast<float>(i * 100),static_cast<float>(r * 100) };
			numbers_[r][i][number]->SetPosition(pos);
			numbers_[r][i][number]->Update();
		}
	}
}

void DamageManager::Draw()
{
	for (size_t r = 0; r < 3; ++r)
	{
		for (size_t i = 0; i < kNumberArray; ++i)
		{
			int number = bitmapNumber_[r][i];
			numbers_[r][i][number]->Draw();
		}
	}
}

void DamageManager::Finalize()
{
	instance.reset();
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

void DamageManager::RankingBitMapFont()
{
	for (int i = 0; i < 3; i++)
	{
		int damage = damageRankings_[i];

		for (int j = 5; j >= 0; j--)
		{
			// ランキングの数値を分割して記録
			bitmapNumber_[i][j] = damage % 10;
			damage /= 10;
		}
	}
}
