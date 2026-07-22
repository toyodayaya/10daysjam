#include "EventManager.h"
#include <cassert>

std::unique_ptr<EventManager> EventManager::instance = nullptr;


EventManager* EventManager::GetInstance()
{
	if (instance == nullptr)
	{
		instance = std::make_unique<EventManager>();
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
	for (const std::unique_ptr<BaseEvent>& event : events_)
	{
		event->Finalize();
	}

	// インスタンスを解放
	instance.reset();
}
