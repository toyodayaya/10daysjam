#pragma once
#include <d3d12.h>
#include <externals/DirectXTex/DirectXTex.h>
#include"externals/DirectXTex/d3dx12.h"
#include <wrl.h>
#include "DirectXBasis.h"
#include "SrvManager.h"
#include "MathManager.h"
#include "ParticleEmitter.h"
#include <numbers>
#include "Camera.h"
#include <random>

class ParticleManager
{
private:

	struct VertexData
	{
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};


	struct  Material
	{
		Vector4 color;
		int32_t enableLighting;
		float padding[3];
		Matrix4x4 uvTransform;
	};

	struct MaterialData
	{
		std::string textureFilePath;
		uint32_t textureIndex = 0;
	};


	struct ModelData
	{
		std::vector<VertexData> vertices;
		MaterialData material;
	};

	struct Particle
	{
		EulerTransform transform;
		Vector3 velocity;
		Vector4 color;
		float lifeTime;
		float currentTime;
	};

	struct ParticleForGPU
	{
		Matrix4x4 WVP;
		Matrix4x4 World;
		Vector4 Color;
	};


	struct ParticleGroup
	{
		MaterialData materialData;
		std::list<Particle> particles;
		uint32_t srvIndex;
		Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource;
		uint32_t instanceCount;
		ParticleForGPU* instancingData;
		ParticleEmitter::Type type;
		Microsoft::WRL::ComPtr <ID3D12Resource> particlesResource;
		uint32_t uavIndex;
		D3D12_VERTEX_BUFFER_VIEW particlesBufferView;
		std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> uavHandle;
		std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> srvHandle;
		uint32_t vertexIndex;
	};

	struct AccelerationField
	{
		Vector3 acceleration;
		AABB area;
	};

	struct ParticleCS
	{
		Vector3 translate;
		Vector3 scale;
		float lifeTime;
		Vector3 velocity;
		float currentTime;
		Vector4 color;
	};

	struct PerView
	{
		Matrix4x4 viewProjection;
		Matrix4x4 billboardMatrix;
	};

	enum BlendMode
	{
		// ブレンドなし
		kBlendModeNone,
		// 通常ブレンド
		kBlendModeNormal,
		// 加算
		kBlendModeAdd,
		// 減算
		kBlendModeSubstract,
		// 乗算
		kBlendModeMultiply,
		// スクリーン
		kBlendModeScreen,
		// 利用禁止
		kCountOfBlendMode
	};

private:



	// ポインタ
	DirectXBasis* dxBasis_ = nullptr;
	SrvManager* srvManager_ = nullptr;
	Camera* camera = nullptr;

	// パーティクル
	Particle particle;

	// 座標
	Vector3 randomTranslate;

	// モデルデータ
	ModelData modelData;
	ModelData modelDataRing;
	ModelData modelDataCylinder;

	// ルートシグネチャー
	Microsoft::WRL::ComPtr <ID3D12RootSignature> rootSignature;
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	Microsoft::WRL::ComPtr <ID3D12RootSignature> computeRootSignature;
	// ComputePipelineState
	Microsoft::WRL::ComPtr<ID3D12PipelineState> computePipelineState;
	// グラフィックスパイプラインステート
	Microsoft::WRL::ComPtr <ID3D12PipelineState> graphicPipelineState;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicPipelineStateDesc{};
	D3D12_BLEND_DESC blendDesc{};
	// RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// shader
	Microsoft::WRL::ComPtr <IDxcBlob> vertexShaderBlob;
	Microsoft::WRL::ComPtr <IDxcBlob> pixelShaderBlob;
	Microsoft::WRL::ComPtr <IDxcBlob> computeShaderBlob;
	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};


	// 頂点リソース
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResource;
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResourceRing;
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResourceCylinder;

	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	VertexData* vertexDataRing = nullptr;
	VertexData* vertexDataCylinder = nullptr;

	// バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewRing;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewCylinder;
	
	// パーティクルグループコンテナ
	std::unordered_map<std::string, ParticleGroup> particleGroups;

	// SRV
	D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc{};


	const float kDeltaTime = 1.0f / 60.0f;

	// 効果範囲
	AccelerationField accelerationField;

	// パーティクル最大数
	const uint32_t kMaxInstanceCount = 10;

	// 乱数生成器
	std::random_device seedGenerator;
	std::mt19937 randomEngine;

	// ビルボードのフラグ
	bool isBillboard = false;
	// ブレンドモード
	BlendMode blendMode_ = kBlendModeAdd;

	// perView
	Microsoft::WRL::ComPtr<ID3D12Resource> perViewResource;
	PerView* perView = nullptr;

public:
	// シングルトンインスタンスの取得
	static ParticleManager* GetInstance();
	// 終了
	void Finalize();
	// 初期化
	void Initialize(DirectXBasis* dxBasis, SrvManager* srvManager, Camera* camera);
	// 更新
	void Update();
	// 描画
	void Draw();

	// ルートシグネチャーの作成
	void CreateRootSignature();
	// グラフィックスパイプラインの生成
	void GenerateGraphicsPipeline();
	// 頂点データ作成
	void CreateVertexData();
	// ブレンドモード設定
	void BlendModeSetting();
	// ComputeShader用のパイプラインステートの生成
	void GenerateCSPipelineState();
	// ComputeShader用のルートシグネチャーの作成
	void CreateCSRootSignature();
	// 共通描画設定
	void DrawSettingCompute();
	// UAVの生成
	void CreateUav();
	// PerViewResourceの生成
	void CreatePerViewResource();

	// パーティクルの作成
	// NormalParticle生成関数
	Particle MakeNewNormalParticle(const Vector3& translate, const Vector3& scale, const Vector3& rotate,
		const Vector3& velocity, const Vector4& color, const float lifeTime, const float currentTime);
	Particle MakeNewHitEffectParticle(const Vector3& translate, const Vector3& scale, const Vector3& rotate,
		const Vector3& velocity, const Vector4& color, const float lifeTime, const float currentTime);
	Particle MakeNewRingParticle(const Vector3& translate, const Vector3& scale, const Vector3& rotate,
		const Vector3& velocity, const Vector4& color, const float lifeTime, const float currentTime);
	Particle MakeNewCylinderParticle(const Vector3& translate, const Vector3& scale, const Vector3& rotate,
		const Vector3& velocity, const Vector4& color, const float lifeTime, const float currentTime);

	// パーティクルグループの生成
	void CreateParticleGroup(const std::string name, const std::string textureFilePath, ParticleEmitter::Type type);

	// パーティクルの発生
	void Emit(const std::string name, const Vector3& translate, const Vector3& scale, const Vector3& rotate,
		const Vector3& velocity, const Vector4& color, const float lifeTime, const float currentTime, uint32_t count);

	// 効果範囲の当たり判定
	bool IsCollision(const AABB& aabb, const Vector3& point);



	ParticleManager() = default;
	~ParticleManager() = default;
	ParticleManager(ParticleManager&) = delete;
	ParticleManager& operator=(ParticleManager&) = delete;
	static std::unique_ptr<ParticleManager> instance;

};