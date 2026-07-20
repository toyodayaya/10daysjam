#pragma once
#include "Object3d.h"
#include <memory>

class Enemy
{
public:
	// 初期化
	void Initialize();
	// 更新
	void Update();
	// 描画
	void Draw();

	// getter
	Object3d* GetObject3d() { return object3d.get(); }

private:
	// 3dオブジェクト
	std::unique_ptr<Object3d> object3d;
};

