//#pragma once
//#include <string>
//#include <map>
//#include "DirectXBasis.h"
//#include <LevelData.h>
//
//class StageManager
//{
//private:
//	// レベルデータ
//	std::map<std::string, std::unique_ptr<LevelData>> levelDatas;
//
//public:
//	static std::unique_ptr<StageManager> instance;
//
//	StageManager() = default;
//	~StageManager() = default;
//	StageManager(StageManager&) = delete;
//	StageManager& operator = (StageManager&) = delete;
//
//	// シングルトンインスタンスの取得
//	static StageManager* GetInstance();
//	// 終了
//	void Finalize();
//	// 初期化
//	void Initialize(DirectXBasis* dxBasis);
//
//	// Jsonデータ読み込み
//	void LoadJsonData(const std::string& directoryPath, const std::string& filePath);
//	// Jsonデータ取得
//	LevelData* FindJsonData(const std::string& filePath);
//
//
//};