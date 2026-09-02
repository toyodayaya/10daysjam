#pragma once
#include "BaseEvent.h"
#include "EventManager.h"
#include "Object3dCommon.h"
#include "Object3d.h"
#include <cstdint>
#include <memory>
#ifdef _DEBUG
#include "DebugDraw.h"
#endif // _DEBUG

class LightHouse : public BaseEvent
{
public:
	// 初期化
	void Initialize(const QuaternionTransform& transform, const std::string& filePath);
	// 終了
	void Finalize() override;
	// 更新
	void Update() override;
	// 描画
	void Draw() override;
	// 衝突応答
	void OnCollision(std::string hitObjectType, BaseCharacter* hitObject) override;
	// 保有中HPを増やす
	void AddHp(uint32_t hp);
	// 保有中HPを取得
	uint32_t GetHp() const { return hp_; }

private:
#ifdef _DEBUG
	std::unique_ptr<DebugDraw> debugDraw;
#endif // _DEBUG

	// 保有中HP
	uint32_t hp_ = 0;

};

