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
	// 保有中HPを取得
	uint32_t GetHp() const { return static_cast<uint32_t>(intencity); }

	// HP加算関数
	void AddHP(const float& hp) override;
	// 最大HPを設定
	void SetMaxHP(const float& hp) override;

	// setter
	void SetIsHit(bool isHit) { this->isHit_ = isHit; }

private:
	// 灯台の明るさ
	float intencity = 10.0f;
	// 灯台の明るさの減衰率
	float decay = 10.0f;
	// 灯台の明るさの限界値
	float maxIntencity = 10.0f;

	// 接触フラグ
	bool isHit_ = false;
	// 線形補間用の変数
	float t = 0.0f;

#ifdef _DEBUG
	std::unique_ptr<DebugDraw> debugDraw;
#endif // _DEBUG

};

