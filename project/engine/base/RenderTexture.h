#pragma once
#include "DirectXBasis.h"
#include "Camera.h"
#include "SrvManager.h"

class RenderTexture
{
private:
	// コンストラクタ
	RenderTexture() = default;
	// デストラクタ
	~RenderTexture() = default;
	// コピーコンストラクタとコピー代入演算子を削除
	RenderTexture(const RenderTexture&) = delete;
	RenderTexture& operator=(const RenderTexture&) = delete;
	// インスタンス
	static RenderTexture* instance;

public:
	struct Material
	{
		Matrix4x4 projectionInverse;
	};

	struct MaterialTime
	{
		float time;
	};

	// 初期化
	void Initialize(DirectXBasis* directXBasis, SrvManager* srvManager);
	// ルートシグネチャーの作成
	void CreateRootSignature();
	// グラフィックスパイプラインの生成
	void GenerateGraphicsPipeline();
	// 共通描画設定
	void DrawSettingCommon();
	// projectionInverseの作成
	void CreateProjectionInverse();
	// 経過時間の作成
	void CreateMaterialTime();
	// getter
	DirectXBasis* GetDxBasis() const { return dxBasis_; }
	Camera* GetDefaultCamera() const { return defaultCamera_; }
	Microsoft::WRL::ComPtr <ID3D12Resource> GetTexture() const { return texture_; }
	// setter
	void SetDefaultCamera(Camera* camera) { this->defaultCamera_ = camera; }

	// インスタンス
	static RenderTexture* GetInstance();
	// 終了
	void Finalize();

private:
	// ポインタ
	DirectXBasis* dxBasis_ = nullptr;
	SrvManager* srvManager_ = nullptr;
	// ルートシグネチャー
	Microsoft::WRL::ComPtr <ID3D12RootSignature> rootSignature_;
	// グラフィックスパイプラインステート
	Microsoft::WRL::ComPtr <ID3D12PipelineState> graphicPipelineState_;
	// BlendStateの設定
	D3D12_BLEND_DESC blendDesc_{};
	// デフォルトカメラ
	Camera* defaultCamera_ = nullptr;

	// WVP用のリソースを作る
	Microsoft::WRL::ComPtr <ID3D12Resource> projecttionInverseResource_;
	// データを書き込む
	Material* projectionInverseData_ = nullptr;

	// テクスチャ
	Microsoft::WRL::ComPtr <ID3D12Resource> texture_;
	// GPUハンドル
	D3D12_GPU_DESCRIPTOR_HANDLE handle_;
	D3D12_GPU_DESCRIPTOR_HANDLE depthHandle_;
	D3D12_GPU_DESCRIPTOR_HANDLE dissolveHandle_;
	// ファイルパス
	std::string filePath_;
	std::string dissolveFilePath_;

	// 経過時間
	Microsoft::WRL::ComPtr <ID3D12Resource> materialTimeResource_;
	MaterialTime* timeData_ = nullptr;
	const float kDeltaTime = 1.0f / 60.0f;
};

