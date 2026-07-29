#pragma once
#include "DirectXBasis.h"
#include "SrvManager.h"
#include "ModelCommon.h"
#include "Camera.h"
#include <wrl.h>

class AnimationCommon
{
public:
	// コンストラクタに渡すための鍵
	class ConstructorKey
	{
	private:
		ConstructorKey() = default;
		friend class AnimationCommon;
	};

	// PassKeyを受け取るコンストラクタ
	explicit AnimationCommon(ConstructorKey){}

private:
	// デストラクタ
	~AnimationCommon() = default;
	// コピーコンストラクタとコピー代入演算子を削除
	AnimationCommon(const AnimationCommon&) = delete;
	AnimationCommon& operator=(const AnimationCommon&) = delete;
	// インスタンス
	friend std::default_delete<AnimationCommon>;
	static std::unique_ptr<AnimationCommon> instance;

private:
	// ポインタ
	DirectXBasis* dxBasis_ = nullptr;
	SrvManager* srvManager_ = nullptr;
	ModelCommon* modelManager = nullptr;
	Camera* camera = nullptr;
	// デフォルトカメラ
	Camera* defaultCamera_ = nullptr;

	// ルートシグネチャー
	Microsoft::WRL::ComPtr <ID3D12RootSignature> computeRootSignature;
	// ComputePipelineState
	Microsoft::WRL::ComPtr<ID3D12PipelineState> computePipelineState;
	// BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicPipelineStateDesc{};
	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	Microsoft::WRL::ComPtr <IDxcBlob> vertexShaderBlob;
	Microsoft::WRL::ComPtr <IDxcBlob> pixelShaderBlob;
	Microsoft::WRL::ComPtr <IDxcBlob> computeShaderBlob;
	std::array<D3D12_INPUT_ELEMENT_DESC, 5> inputElementDescs{};
	// RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};

public:
	// 初期化
	void Initialize(DirectXBasis* directXBasis, SrvManager* srvManager);
	// ComputeShader用のパイプラインステートの生成
	void GenerateCSPipelineState();
	// ComputeShader用のルートシグネチャーの作成
	void CreateCSRootSignature();
	// 共通描画設定
	void DrawSettingCompute();

	// getter
	DirectXBasis* GetDxBasis() const { return dxBasis_; }
	SrvManager* GetSrvManager() const { return srvManager_; }
	Camera* GetDefaultCamera() const { return defaultCamera_; }
	// setter
	void SetDefaultCamera(Camera* camera) { this->defaultCamera_ = camera; }

	// インスタンス
	static AnimationCommon* GetInstance();
	// 終了
	void Finalize();
};

