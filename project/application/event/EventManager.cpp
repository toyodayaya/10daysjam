#include "EventManager.h"
#include "LightHouse.h"
#include <cassert>

std::unique_ptr<EventManager> EventManager::instance = nullptr;


EventManager* EventManager::GetInstance()
{
	if (instance == nullptr)
	{
		instance = std::make_unique<EventManager>(ConstructorKey());
	}
	return instance.get();
}

void EventManager::Update()
{
	// 登録されたイベントを更新
	for (const std::unique_ptr<BaseEvent>& event : events_)
	{
		event->Update();
	}
}

void EventManager::Draw()
{
	// 登録されたイベントを描画
	for (const std::unique_ptr<BaseEvent>& event : events_)
	{
		event->Draw();
	}
}

void EventManager::Finalize()
{
	// 登録されたイベントを解放
	events_.clear();

	// インスタンスを解放
	instance.reset();
}

LightHouse* EventManager::GetHighestHpLightHouse() const
{
	LightHouse* highestHpLightHouse = nullptr;

	for (const std::unique_ptr<BaseEvent>& event : events_)
	{
		LightHouse* lightHouse = dynamic_cast<LightHouse*>(event.get());
		if (lightHouse != nullptr &&
			(highestHpLightHouse == nullptr || lightHouse->GetHp() > highestHpLightHouse->GetHp()))
		{
			highestHpLightHouse = lightHouse;
		}
	}

	return highestHpLightHouse;
}
