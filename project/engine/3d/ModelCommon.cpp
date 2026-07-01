#include "ModelCommon.h"

void ModelCommon::Initialize(DirectXBasis* directXBasis, SrvManager* srvManager)
{
	// 引数で受け取ってメンバ変数として記録する
	dxBasis_ = directXBasis;
	srvManager_ = srvManager;
}