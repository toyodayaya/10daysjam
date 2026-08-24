#pragma once
#include <memory>
#include "Object3d.h"
#include "MathManager.h"
using namespace MathManager;

class Skydome
{
public:
	// 初期化
	void Initialize(const QuaternionTransform& transform, std::string textureFilePath);
	// 更新
	void Update();
	// 描画
	void Draw();
	// 終了
	void Finalize();

private:
	// 3Dオブジェクト
	std::unique_ptr<Object3d> object3d_;
	// トランスフォーム
	QuaternionTransform transform_;
};

