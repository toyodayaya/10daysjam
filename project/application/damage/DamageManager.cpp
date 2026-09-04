#include "DamageManager.h"
#include "TextureManager.h"
#include "SpriteCommon.h"
#include <cassert>
#include <random>
#include "Input.h"
#include "MathManager.h"
using namespace MathManager;

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
	for (size_t r = 0; r < 4; ++r)
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
			numbers_[r][i][number]->Update();
		}
	}
}

void DamageManager::BestDamageUpdate()
{
	switch (phase_)
	{
	case Roll:
	{
		if (drumRollTimer_ <= 0.0f)
		{
			phase_ = Notice;
			return;
		}
		else
		{
			if (Input::GetInstance()->TriggerKey(DIK_SPACE))
			{
				drumRollTimer_ = 0.0f;
				return;
			}

			for (size_t i = 0; i < kNumberArray; ++i)
			{
				for (size_t j = 0; j < 10; j++)
				{
					// ビットマップフォントの位置を設定
					numbers_[3][i][j]->Update();
				}
			}

			drumRollTimer_ -= kDeltaTime;
		}


		break;
	}

	case Notice:
	{

		for (size_t i = 0; i < kNumberArray; ++i)
		{
			int number = bitmapNumber_[3][i];
			numbers_[3][i][number]->Update();
		}

		break;
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

void DamageManager::BestDamageDraw()
{
	switch (phase_)
	{
	case Roll:
	{
		// ドラムロール用にランダム値を初期化
		std::random_device seedGenerator;
		std::mt19937 randomEngine(seedGenerator());

		for (size_t i = 0; i < kNumberArray; ++i)
		{
			// ランダム値を算出
			std::uniform_int_distribution distribution(0, 9);
			int randomNumber_ = distribution(randomEngine);
			numbers_[3][i][randomNumber_]->Draw();
		}
		break;
	}
	case Notice:
	{

		for (size_t i = 0; i < kNumberArray; ++i)
		{
			int number = bitmapNumber_[3][i];
			numbers_[3][i][number]->Draw();
		}

		break;
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
	for (int r = 0; r < 3; r++)
	{
		int damage = damageRankings_[r];

		for (int i = 5; i >= 0; i--)
		{
			// ランキングの数値を分割して記録
			bitmapNumber_[r][i] = damage % 10;
			damage /= 10;
		}

		// ビットマップフォントの位置を設定
		for (size_t i = 0; i < kNumberArray; ++i)
		{
			for (int j = 0; j < 10; j++)
			{
				int number = bitmapNumber_[r][i];
				Vector2 pos = { static_cast<float>(i * 100),static_cast<float>(r * 100) };
				numbers_[r][i][number]->SetPosition(pos);
			}
		}
	}
}

void DamageManager::BestDamageBitMapFont()
{
	for (int j = 5; j >= 0; j--)
	{
		// 最高ダメージの数値を分割して記録
		bitmapNumber_[3][j] = bestDamage_ % 10;
		bestDamage_ /= 10;
	}

	// ビットマップフォントの位置を設定
	for (size_t i = 0; i < kNumberArray; ++i)
	{
		for (size_t j = 0; j < 10; j++)
		{
			Vector2 pos = { static_cast<float>(i * 100),0.0f };
			numbers_[3][i][j]->SetPosition(pos);
		}

	}
}

void DamageManager::SetOnePlayBestDamage(const int& damage)
{
	// 引数のダメージと記録内の最大ダメージを比較
	if (bestDamage_ <= damage)
	{
		// 引数のダメージの方が大きかったら記録
		bestDamage_ = damage;
	}

	// ランキングを更新
	RankingUpdate(damage);
}
