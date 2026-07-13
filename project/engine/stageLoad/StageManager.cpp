//#include "StageManager.h"
//#include "LevelData.h"
//
//std::unique_ptr<StageManager> StageManager::instance = nullptr;
//
//StageManager* StageManager::GetInstance()
//{
//	if (instance == nullptr)
//	{
//		instance = std::make_unique<StageManager>();
//	}
//
//	return instance.get();
//}
//
//void StageManager::Finalize()
//{
//	instance.reset();
//}
//
//void StageManager::Initialize(DirectXBasis* dxBasis)
//{
//	
//}
//
//void StageManager::LoadJsonData(const std::string& directoryPath, const std::string& filePath)
//{
//	// 読み込み済みJsonデータを検索
//	if (levelDatas.contains(filePath))
//	{
//		return;
//	}
//
//	// Jsonデータの生成とファイル読み込み、初期化
//	std::unique_ptr<LevelData> model = std::make_unique<LevelData>();
//	//model->Initialize(modelCommon.get(), directoryPath, filePath, type);
//
//	// Jsonデータをmapコンテナに格納する
//	levelDatas.insert(std::make_pair(filePath, std::move(model)));
//}
//
//LevelData* StageManager::FindJsonData(const std::string& filePath)
//{
//	// 読み込み済みJsonデータを検索
//	if (levelDatas.contains(filePath))
//	{
//		// 読み込み済みJsonデータを戻り値としてreturn
//		return levelDatas.at(filePath).get();
//	}
//
//	// ファイル名一致なし
//	return nullptr;
//}
