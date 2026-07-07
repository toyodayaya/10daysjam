#include "SceneFactory.h"
#include "TitleScene.h"
#include "GamePlayScene.h"

std::unique_ptr<BaseScene> SceneFactory::CreateScene(const std::string& sceneName)
{
    // 次のシーンを生成
	std::unique_ptr<BaseScene> nextScene;

	if(sceneName == "TitleScene")
	{
		nextScene = std::make_unique<TitleScene>();
	}
	else if(sceneName == "GamePlayScene")
	{
		nextScene = std::make_unique<GamePlayScene>();
	}

	return nextScene;
}
