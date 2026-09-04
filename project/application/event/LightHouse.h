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
	uint32_t GetHp() const { return hp_; }
	// 使用中・破壊演出中かを取得
	bool IsHit() const { return isHit_; }

	// HP加算関数
	void AddHP(const float& hp) override;
	// 最大HPを設定
	void SetMaxHP(const float& hp) override;
	// 最大maxAmountまで灯台のHPを取り出す
	uint32_t WithdrawHp(uint32_t maxAmount);
	// 灯台本体の押し出し判定用AABB
	AABB GetCollisionAabb() const;
	// PlayerがHPを受け渡すための広めのAABB
	AABB GetInteractionAabb() const;

	// setter
	void SetIsHit(bool isHit);
	void SetHp(uint32_t hp) { this->hp_ = hp; }

private:
	// ゲーム上で灯台が保有しているHP
	uint32_t hp_ = 10;
	// 灯台の明るさ
	float intencity = 0.0f;
	// 灯台の明るさの減衰率
	float decay = 10.0f;

	// 接触フラグ
	bool isHit_ = false;
	// 線形補間用の変数
	float t = 0.0f;
	// 灯台本体と、HP受け渡し範囲の大きさ
	const Vector3 kCollisionAabbHalfSize_ = { 1.0f, 1.0f, 1.0f };
	const Vector3 kInteractionAabbHalfSize_ = { 2.0f, 2.0f, 2.0f };

#ifdef _DEBUG
	std::unique_ptr<DebugDraw> debugDraw;
#endif // _DEBUG

};

