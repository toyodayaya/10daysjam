#pragma once
#include "BaseEvent.h"
#include <memory>
#include <vector>

class EventManager
{
public:
	// コンストラクタ
	EventManager() = default;
	// デストラクタ
	~EventManager() = default;
	// コピーコンストラクタとコピー代入演算子を削除
	EventManager(const EventManager&) = delete;
	EventManager& operator=(const EventManager&) = delete;
	// インスタンス
	static std::unique_ptr<EventManager> instance;
	
private:
	// 登録済みイベントを記録する配列
	std::vector<std::unique_ptr<BaseEvent>> events_;

public:
	// 更新
	void Update();
	// 描画
	void Draw();
	// 終了
	void Finalize();


	// setter
	void SetEvents(std::unique_ptr<BaseEvent> event) { events_.push_back(std::move(event)); }
	
	// インスタンス
	static EventManager* GetInstance();


};

