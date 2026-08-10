#include "StageManager.h"
#include "StageData.h"
#include "EventManager.h"
#include "EnemyManager.h"
#include "BulletManager.h"

std::unique_ptr<StageManager> StageManager::instance = nullptr;

StageManager* StageManager::GetInstance()
{
	if (instance == nullptr)
	{
		instance = std::make_unique<StageManager>(ConstructorKey());
	}

	return instance.get();
}

void StageManager::Finalize()
{
	// イベントマネージャーの終了
	EventManager::GetInstance()->Finalize();
	// 敵マネージャーの終了
	EnemyManager::GetInstance()->Finalize();
	// 弾マネージャーの終了
	BulletManager::GetInstance()->Finalize();

	instance.reset();
}

void StageManager::Initialize()
{
	
}

void StageManager::LoadJsonData(const std::string& directoryPath, const std::string& filePath)
{
	// 読み込み済みJsonデータを検索
	if (stageDatas.contains(filePath))
	{
		return;
	}

	// Jsonデータの生成とファイル読み込み、初期化
	std::unique_ptr<StageData> stageData = std::make_unique<StageData>();
	stageData->Initialize(directoryPath, filePath,StageManager::GetInstance());

	// Jsonデータをmapコンテナに格納する
	stageDatas.insert(std::make_pair(filePath, std::move(stageData)));
}

StageData* StageManager::FindJsonData(const std::string& filePath)
{
	// 読み込み済みJsonデータを検索
	if (stageDatas.contains(filePath))
	{
		// 読み込み済みJsonデータを戻り値としてreturn
		return stageDatas.at(filePath).get();
	}

	// ファイル名一致なし
	return nullptr;
}
