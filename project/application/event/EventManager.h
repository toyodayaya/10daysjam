#pragma once
#include "BaseEvent.h"
#include <memory>
#include <vector>

class LightHouse;

class EventManager
{
public:
	// コンストラクタに渡すための鍵
	class ConstructorKey
	{
	private:
		ConstructorKey() = default;
		friend class EventManager;
	};

	// PassKeyを受け取るコンストラクタ
	explicit EventManager(ConstructorKey) {}
	
	
private:
	// デストラクタ
	~EventManager() = default;
	// コピーコンストラクタとコピー代入演算子を削除
	EventManager(const EventManager&) = delete;
	EventManager& operator=(const EventManager&) = delete;
	// インスタンス
	friend std::default_delete<EventManager>;
	static std::unique_ptr<EventManager> instance;

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
	// 保有中HPが最も多い灯台を取得
	LightHouse* GetHighestHpLightHouse() const;
	
	// インスタンス
	static EventManager* GetInstance();


};

