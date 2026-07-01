#pragma once
#include "DirectXBasis.h"
#include "SrvManager.h"

class ModelCommon
{
public:
	// 初期化
	void Initialize(DirectXBasis* directXBasis,SrvManager* srvManager);

	// getter
	DirectXBasis* GetDxBasis() const { return dxBasis_; }
	SrvManager* GetSrvManager() const { return srvManager_; }

private:
	// ポインタ
	DirectXBasis* dxBasis_ = nullptr;
	SrvManager* srvManager_ = nullptr;
};

