#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include "ModelCommon.h"
#include "DirectXBasis.h"
#include "SrvManager.h"
#include "Model.h"

class ModelManager
{
private:
	// モデルデータ
	std::map<std::string, std::unique_ptr<Model>> models;

	// モデル共通部のポインタ
	std::unique_ptr <ModelCommon> modelCommon;

public:
	static std::unique_ptr<ModelManager> instance;

	ModelManager() = default;
	~ModelManager() = default;
	ModelManager(ModelManager&) = delete;
	ModelManager& operator = (ModelManager&) = delete;

	// シングルトンインスタンスの取得
	static ModelManager* GetInstance();
	// 終了
	void Finalize();
	// 初期化
	void Initialize(DirectXBasis* dxBasis,SrvManager* srvManager);

	// モデルファイル読み込み
	void LoadModel(const std::string& directoryPath, const std::string& filePath, Model::AnimationType type);
	// モデルデータ取得
	Model* FindModel(const std::string& filePath);
};
