#include "SceneFactory.h"
#include "TitleScene.h"
#include "GamePlayScene.h"
#include "ResultScene.h"
#include "TutorialScene.h"


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
	else if (sceneName == "ResultScene")
	{
		nextScene = std::make_unique<ResultScene>();
	}
	else if (sceneName == "TutorialScene")
	{
		nextScene = std::make_unique<TutorialScene>();
	}

	return nextScene;
}
