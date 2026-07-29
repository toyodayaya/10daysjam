#pragma once
#include "BaseScene.h"
#include "SceneFactory.h"
#include <memory>

class SceneManager
{
public:
	// コンストラクタに渡すための鍵
	class ConstructorKey
	{
	private:
		ConstructorKey() = default;
		friend class SceneManager;
	};

	// PassKeyを受け取るコンストラクタ
	explicit SceneManager(ConstructorKey) {}

private:

	// デストラクタ
	~SceneManager() = default;
	// コピーコンストラクタとコピー代入演算子を削除
	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;
	// インスタンス
	friend std::default_delete<SceneManager>;
	static std::unique_ptr<SceneManager> instance;

public:
	// 次シーン予約
	void ChangeScene(const std::string& sceneName);
	// 更新
	void Update();
	// 描画
	void Draw();
	// 終了
	void Finalize();

	// インスタンス
	static SceneManager* GetInstance();

	// シーンファクトリーのセット
	void SetSceneFactory(std::unique_ptr<AbstractSceneFactory> sceneFactory) { sceneFactory_ = std::move(sceneFactory); }

private:
	// 実行中のシーン
	std::unique_ptr<BaseScene> scene_;
	// 次シーン
	std::unique_ptr<BaseScene> nextScene_ = nullptr;
	// シーンファクトリー
	std::unique_ptr<AbstractSceneFactory> sceneFactory_ = nullptr;
};

