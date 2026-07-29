#pragma once
#include "DirectXBasis.h"
#include "Camera.h"
#include "SrvManager.h"

class RenderTexture
{
public:
	// コンストラクタに渡すための鍵
	class ConstructorKey
	{
	private:
		ConstructorKey() = default;
		friend class RenderTexture;
	};

	// PassKeyを受け取るコンストラクタ
	explicit RenderTexture(ConstructorKey) {}

private:

	// デストラクタ
	~RenderTexture() = default;
	// コピーコンストラクタとコピー代入演算子を削除
	RenderTexture(const RenderTexture&) = delete;
	RenderTexture& operator=(const RenderTexture&) = delete;
	// インスタンス
	friend std::default_delete<RenderTexture>;
	static std::unique_ptr<RenderTexture> instance;

public:
	struct Material
	{
		Matrix4x4 projectionInverse;
	};

	struct MaterialTime
	{
		float time;
	};

	// ポストエフェクトのタイプ
	enum PostEffect
	{
		kNormal,
		kGrayScale,
		kSepiaScale,
		kVignetting,
		kBoxFilter,
		kGaussianFilter,
		kOutline,
		kRadialBlur,
		kDissolve,
		kRandom
	};

	// 初期化
	void Initialize(DirectXBasis* directXBasis, SrvManager* srvManager);
	// ルートシグネチャーの作成
	void CreateRootSignature();
	// グラフィックスパイプラインの生成
	void GenerateGraphicsPipeline();
	// 各シェーダー用の設定
	void GeneratePostEffect();
	// シェーダーの生成
	// Grayscaleの生成
	void GenerateGrayScale();
	// Sepiaの生成
	void GenerateSepia();
	// Vignettingの生成
	void GenerateVignetting();
	// BoxFilterの生成
	void GenerateBoxFilter();
	// GaussianFilterの生成
	void GenerateGaussianFilter();
	// Outlineの生成
	void GenerateOutline();
	// RadialBlurの生成
	void GenerateRadialBlur();
	// Dissolveの生成
	void GenerateDissolve();
	// Randomの生成
	void GenerateRandom();

	// 共通描画設定
	void DrawSettingCommon();
	// PSO設定
	void DrawSettingPSO();
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
	void SetPostEffect(PostEffect type) { this->type_ = type; }
	void SetDissolveTexture(std::string filePath) { dissolveFilePath_ = filePath; }

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

	// 各シェーダーのPSO
	// normal
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicPipelineStateDesc{};
	Microsoft::WRL::ComPtr <ID3D12PipelineState> graphicPipelineState_;
	// Grayscale
	D3D12_GRAPHICS_PIPELINE_STATE_DESC grayscaleGPSD_{};
	Microsoft::WRL::ComPtr <ID3D12PipelineState> grayscaleGPS_;
	// Sepiascale
	D3D12_GRAPHICS_PIPELINE_STATE_DESC sepiascaleGPSD_{};
	Microsoft::WRL::ComPtr <ID3D12PipelineState> sepiascaleGPS_;
	// Vignetting
	D3D12_GRAPHICS_PIPELINE_STATE_DESC vignettingGPSD_{};
	Microsoft::WRL::ComPtr <ID3D12PipelineState> vignettingGPS_;
	// BoxFilter
	D3D12_GRAPHICS_PIPELINE_STATE_DESC boxFilterGPSD_{};
	Microsoft::WRL::ComPtr <ID3D12PipelineState> boxFilterGPS_;
	// GaussianFilter
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gaussianFilterGPSD_{};
	Microsoft::WRL::ComPtr <ID3D12PipelineState> gaussianFilterGPS_;
	// OutLine
	D3D12_GRAPHICS_PIPELINE_STATE_DESC outlineGPSD_{};
	Microsoft::WRL::ComPtr <ID3D12PipelineState> outlineGPS_;
	// RadialBlur
	D3D12_GRAPHICS_PIPELINE_STATE_DESC radialBlurGPSD_{};
	Microsoft::WRL::ComPtr <ID3D12PipelineState> radialBlurGPS_;
	// Dissolve
	D3D12_GRAPHICS_PIPELINE_STATE_DESC dissolveGPSD_{};
	Microsoft::WRL::ComPtr <ID3D12PipelineState> dissolveGPS_;
	// Random
	D3D12_GRAPHICS_PIPELINE_STATE_DESC randomGPSD_{};
	Microsoft::WRL::ComPtr <ID3D12PipelineState> randomGPS_;

	// 使用するシェーダーのタイプ
	PostEffect type_ = kNormal;
};


