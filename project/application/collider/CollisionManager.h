#pragma once
#include <memory>
#include "MathManager.h"
using namespace MathManager;

class CollisionManager
{
public:
	
	// コンストラクタに渡すための鍵
	class ConstructorKey
	{
	private:
		ConstructorKey() = default;
		friend class CollisionManager;
	};

	// PassKeyを受け取るコンストラクタ
	explicit CollisionManager(ConstructorKey) {}
	
private:
	// デストラクタ
	~CollisionManager() = default;
	// コピーコンストラクタとコピー代入演算子を削除
	CollisionManager(const CollisionManager&) = delete;
	CollisionManager& operator=(const CollisionManager&) = delete;
	// インスタンス
	friend std::default_delete<CollisionManager>;
	static std::unique_ptr<CollisionManager> instance;

public:
	// 当たり判定の関数
	bool IsCollision(const AABB& aabb, const AABB& aabbHit);
	// getter
	AABB MakeAABB(const Vector3& translate,const Vector3& size);
	// インスタンス
	static CollisionManager* GetInstance();
};

