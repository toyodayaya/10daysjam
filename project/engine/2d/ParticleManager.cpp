#include "ParticleManager.h"
#include "DirectXBasis.h"
#include "Logger.h"
#include <random>
#include <cassert>
#include "MathManager.h"
#include "TextureManager.h"
#include "ImguiManager.h"

using namespace Logger;
using namespace MathManager;

std::unique_ptr<ParticleManager> ParticleManager::instance = nullptr;

ParticleManager* ParticleManager::GetInstance()
{
	if (instance == nullptr)
	{
		instance = std::unique_ptr<ParticleManager>(new ParticleManager);
	}

	return instance.get();
}

void ParticleManager::Finalize()
{
	instance.reset();
}

void ParticleManager::Initialize(DirectXBasis* dxBasis, SrvManager* srvManager, Camera* camera)
{
	// 引数で受け取ってメンバ変数として記録する
	this->dxBasis_ = dxBasis;
	this->srvManager_ = srvManager;
	this->camera = camera;

	// ランダムエンジンの初期化
	std::random_device seedGenerator;
	std::mt19937 randomEngine(seedGenerator());


	// グラフィックスパイプラインの生成
	GenerateGraphicsPipeline();

	// CSパイプラインの生成
	GenerateCSPipelineState();

	// 頂点データ作成
	CreateVertexData();

	// マテリアルリソースの作成
	CreateMaterialResource();

	// UAVの生成
	CreateUav();

	// perViewResourceの生成
	CreatePerViewResource();

	// perFrameResourceの生成
	CreatePerFrameResource();

	// EmitterResourceの生成
	CreateEmitterResource();

	// 効果範囲の設定
	accelerationField.acceleration = Vector3(0.0f, 0.0f, 0.0f);
	accelerationField.area.min = Vector3(-1.0f, -1.0f, -1.0f);
	accelerationField.area.max = Vector3(1.0f, 1.0f, 1.0f);

	// emitterの初期値を設定
	emitterSphere->count = 10;
	emitterSphere->frequency = 0.5f;
	emitterSphere->frequencyTime = 0.0f;
	emitterSphere->translate = Vector3(0.0f, 0.0f, 0.0f);
	emitterSphere->radius = 1.0f;
	emitterSphere->emit = 0;

	perFrame->time = 1.0f;
}

void ParticleManager::CreateRootSignature()
{
	// RootSignatureを作成
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;// 0から始まる
	descriptorRange[0].NumDescriptors = 1; // 数は1つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// RootParameterを作成
	D3D12_ROOT_PARAMETER rootParameters[4] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	rootParameters[0].Descriptor.ShaderRegister = 0; // レジスタ番号0とバインド

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // VertexShaderで使う
	rootParameters[1].Descriptor.ShaderRegister = 0; // レジスタ番号0とバインド
	rootParameters[1].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PSで使う
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // PixelShaderで使う
	rootParameters[3].Descriptor.ShaderRegister = 0; // レジスタ番号0とバインド

	descriptionRootSignature.pParameters = rootParameters; // ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters); // 配列の長さ

	// Samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	// シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr <ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr <ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr))
	{
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリを元に生成
	hr = dxBasis_->GetDevice()->CreateRootSignature(0,
		signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));


}

void ParticleManager::GenerateGraphicsPipeline()
{
	// ルートシグネチャーの作成
	CreateRootSignature();

	// InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = { };
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// BlendStateの設定
	// 全ての色要素を書き込む
	BlendModeSetting();

	// RasterizerStateの設定を行う
	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	// カリングしない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;

	// ShaderをCompileする
	vertexShaderBlob = dxBasis_->CompileShader(L"resources/shaders/Particle.VS.hlsl",
		L"vs_6_0");
	assert(vertexShaderBlob != nullptr);

	pixelShaderBlob = dxBasis_->CompileShader(L"resources/shaders/Particle.PS.hlsl",
		L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	// DepthStencilStateの設定
	// Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	// 書き込む
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	// 比較関数
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	// PSOを生成する
	graphicPipelineStateDesc.pRootSignature = rootSignature.Get();
	graphicPipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicPipelineStateDesc.VS =
	{
		vertexShaderBlob->GetBufferPointer(),
		vertexShaderBlob->GetBufferSize()
	};
	graphicPipelineStateDesc.PS =
	{
		pixelShaderBlob->GetBufferPointer(),
		pixelShaderBlob->GetBufferSize()
	};
	graphicPipelineStateDesc.BlendState = blendDesc;
	graphicPipelineStateDesc.RasterizerState = rasterizerDesc;
	// 書き込むRTVの情報
	graphicPipelineStateDesc.NumRenderTargets = 1;
	graphicPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジのタイプ(三角形)
	graphicPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定
	graphicPipelineStateDesc.SampleDesc.Count = 1;
	graphicPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// DepthStencilの設定
	graphicPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// 生成
	HRESULT hr = dxBasis_->GetDevice()->CreateGraphicsPipelineState(&graphicPipelineStateDesc,
		IID_PPV_ARGS(&graphicPipelineState));

	assert(SUCCEEDED(hr));

}

void ParticleManager::CreateVertexData()
{
	// パーティクルの初期化
	modelData.vertices.push_back({ .position = {1.0f,1.0f,0.0f,1.0f},.texcoord = {0.0f,0.0f},.normal = {0.0f,0.0f,1.0f} });
	modelData.vertices.push_back({ .position = {-1.0f,1.0f,0.0f,1.0f},.texcoord = {1.0f,0.0f},.normal = {0.0f,0.0f,1.0f} });
	modelData.vertices.push_back({ .position = {1.0f,-1.0f,0.0f,1.0f},.texcoord = {0.0f,1.0f},.normal = {0.0f,0.0f,1.0f} });
	modelData.vertices.push_back({ .position = {1.0f,-1.0f,0.0f,1.0f},.texcoord = {0.0f,1.0f},.normal = {0.0f,0.0f,1.0f} });
	modelData.vertices.push_back({ .position = {-1.0f,1.0f,0.0f,1.0f},.texcoord = {1.0f,0.0f},.normal = {0.0f,0.0f,1.0f} });
	modelData.vertices.push_back({ .position = {-1.0f,-1.0f,0.0f,1.0f},.texcoord = {1.0f,1.0f},.normal = {0.0f,0.0f,1.0f} });
	modelData.material.textureFilePath = "resources/circle.png";

	// 頂点リソースを作る
	vertexResource = dxBasis_->CreateBufferResources(sizeof(VertexData) * modelData.vertices.size());

	// 頂点バッファビューを作成する
	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	// 1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// 頂点リソースにデータを書き込む
	// 書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	// 頂点データにリソースをコピー
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());

	// Ringの初期化
	// 分割数
	const uint32_t kRingDivide = 32;
	uint32_t numRingIndices = kRingDivide * 6;
	const float kOuterRadius = 1.0f;
	const float kInnerRadius = 0.2f;
	const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / float(kRingDivide);

	for (uint32_t index = 0; index < kRingDivide; ++index)
	{
		float sin = std::sin(index * radianPerDivide);
		float cos = std::cos(index * radianPerDivide);
		float sinNext = std::sin((index + 1) * radianPerDivide);
		float cosNext = std::cos((index + 1) * radianPerDivide);
		float u = float(index) / float(kRingDivide);
		float uNext = float(index + 1) / float(kRingDivide);
		VertexData p1 = { {cos * kInnerRadius, sin * kInnerRadius, 0.0f,1.0f}, {u, 1.0f} }; // 内
		VertexData p2 = { {cos * kOuterRadius, sin * kOuterRadius, 0.0f,1.0f}, {u, 0.0f} }; // 外
		VertexData p3 = { {cosNext * kInnerRadius, sinNext * kInnerRadius, 0.0f,1.0f}, {uNext, 1.0f} }; // 内(次)
		VertexData p4 = { {cosNext * kOuterRadius, sinNext * kOuterRadius, 0.0f,1.0f}, {uNext, 0.0f} }; // 外(次)

		// 三角形1: p1 (内現) -> p2 (外現) -> p4 (外次)
		modelDataRing.vertices.push_back(p1);
		modelDataRing.vertices.push_back(p2);
		modelDataRing.vertices.push_back(p4);

		// 三角形2: p1 (内現) -> p4 (外次) -> p3 (内次)
		modelDataRing.vertices.push_back(p1);
		modelDataRing.vertices.push_back(p4);
		modelDataRing.vertices.push_back(p3);
	}
	modelDataRing.material.textureFilePath = "resources/gradationLine.png";


	// 頂点リソースを作る
	vertexResourceRing = dxBasis_->CreateBufferResources(sizeof(VertexData) * modelDataRing.vertices.size());

	// 頂点バッファビューを作成する
	// リソースの先頭のアドレスから使う
	vertexBufferViewRing.BufferLocation = vertexResourceRing->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferViewRing.SizeInBytes = UINT(sizeof(VertexData) * modelDataRing.vertices.size());
	// 1頂点あたりのサイズ
	vertexBufferViewRing.StrideInBytes = sizeof(VertexData);

	// 頂点リソースにデータを書き込む
	// 書き込むためのアドレスを取得
	vertexResourceRing->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataRing));
	// 頂点データにリソースをコピー
	std::memcpy(vertexDataRing, modelDataRing.vertices.data(), sizeof(VertexData) * modelDataRing.vertices.size());

	// Cylinderの頂点データ作成
	const uint32_t kCylinderDivide = 32;
	const float kTopRadius = 1.0f;
	const float kBottomRadius = 1.0f;
	const float kHeight = 3.0f;
	const float radianPerDivideCylinder = 2.0f * std::numbers::pi_v<float> / float(kCylinderDivide);

	for (uint32_t index = 0; index < kCylinderDivide; ++index)
	{
		float sin = std::sin(index * radianPerDivideCylinder);
		float cos = std::cos(index * radianPerDivideCylinder);
		float sinNext = std::sin((index + 1) * radianPerDivideCylinder);
		float cosNext = std::cos((index + 1) * radianPerDivideCylinder);
		float u = float(index) / float(kCylinderDivide);
		float uNext = float(index + 1) / float(kCylinderDivide);

		modelDataCylinder.vertices.push_back({ .position = {-sin * kTopRadius,kHeight,cos * kTopRadius,1.0f}, .texcoord = {u, 1.0f} ,.normal = {-sin, 0.0f, cos} });
		modelDataCylinder.vertices.push_back({ .position = {-sinNext * kTopRadius,kHeight,cosNext * kTopRadius,1.0f}, .texcoord = {uNext, 1.0f} ,.normal = {-sinNext, 0.0f, cosNext} });
		modelDataCylinder.vertices.push_back({ .position = {-sin * kBottomRadius,0.0f,cos * kBottomRadius,1.0f}, .texcoord = {u, 0.0f} ,.normal = {-sin, 0.0f, cos} });
		modelDataCylinder.vertices.push_back({ .position = {-sin * kBottomRadius,0.0f,cos * kBottomRadius,1.0f}, .texcoord = {u, 0.0f} ,.normal = {-sin, 1.0f, cos} });
		modelDataCylinder.vertices.push_back({ .position = {-sinNext * kTopRadius,kHeight,cosNext * kTopRadius,1.0f}, .texcoord = {uNext, 1.0f} ,.normal = {-sinNext, 1.0f, cosNext} });
		modelDataCylinder.vertices.push_back({ .position = {-sinNext * kBottomRadius,0.0f,cosNext * kBottomRadius,1.0f}, .texcoord = {uNext, 0.0f} ,.normal = {-sinNext, 1.0f, cosNext} });

	}

	modelDataCylinder.material.textureFilePath = "resources/gradationLine.png";


	// 頂点リソースを作る
	vertexResourceCylinder = dxBasis_->CreateBufferResources(sizeof(VertexData) * modelDataCylinder.vertices.size());

	// 頂点バッファビューを作成する
	// リソースの先頭のアドレスから使う
	vertexBufferViewCylinder.BufferLocation = vertexResourceCylinder->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferViewCylinder.SizeInBytes = UINT(sizeof(VertexData) * modelDataCylinder.vertices.size());
	// 1頂点あたりのサイズ
	vertexBufferViewCylinder.StrideInBytes = sizeof(VertexData);

	// 頂点リソースにデータを書き込む
	// 書き込むためのアドレスを取得
	vertexResourceCylinder->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataCylinder));
	// 頂点データにリソースをコピー
	std::memcpy(vertexDataCylinder, modelDataCylinder.vertices.data(), sizeof(VertexData) * modelDataCylinder.vertices.size());
}

void ParticleManager::BlendModeSetting()
{
	blendDesc.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;

	// ブレンドモードを切り替える
	switch (blendMode_)
	{
	case kBlendModeNormal:
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

		break;

	case kBlendModeAdd:
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;

		break;

	case kBlendModeSubstract:
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;

		break;

	case kBlendModeMultiply:
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;

		break;

	case kBlendModeScreen:
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;

		break;
	}

	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
}

void ParticleManager::GenerateCSPipelineState()
{
	// ルートシグネチャーを作成
	CreateCSRootSignature();

	// shaderをcompileする
	computeShaderBlob = dxBasis_->CompileShader(L"resources/shaders/InitializeParticle.CS.hlsl",
		L"cs_6_0");
	assert(computeShaderBlob != nullptr);

	// CS用のパイプラインステートを設定する
	D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineStateDesc{};
	computePipelineStateDesc.CS =
	{
		.pShaderBytecode = computeShaderBlob->GetBufferPointer(),
		.BytecodeLength = computeShaderBlob->GetBufferSize()
	};

	computePipelineStateDesc.pRootSignature = computeRootSignature.Get();
	HRESULT hr = dxBasis_->GetDevice()->CreateComputePipelineState(&computePipelineStateDesc, IID_PPV_ARGS(&computePipelineState));


	// shaderをcompileする
	computeShaderBlob = ParticleManager::GetInstance()->GetDxBasis()->CompileShader(L"resources/shaders/EmitterParticle.CS.hlsl",
		L"cs_6_0");
	assert(computeShaderBlob != nullptr);

	// CS用のパイプラインステートを設定する
	D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineStateDescEmit{};
	computePipelineStateDescEmit.CS =
	{
		.pShaderBytecode = computeShaderBlob->GetBufferPointer(),
		.BytecodeLength = computeShaderBlob->GetBufferSize()
	};

	computePipelineStateDescEmit.pRootSignature = computeRootSignature.Get();
	hr = dxBasis_->GetDevice()->CreateComputePipelineState(&computePipelineStateDescEmit, IID_PPV_ARGS(&computePipelineStateEmit));

}

void ParticleManager::CreateCSRootSignature()
{
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// Particle用のUAVのDescriptorRange
	D3D12_DESCRIPTOR_RANGE descriptorRangeParticle[1] = {};
	descriptorRangeParticle[0].BaseShaderRegister = 0;// u0
	descriptorRangeParticle[0].NumDescriptors = 1; // 数は1つ
	descriptorRangeParticle[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	descriptorRangeParticle[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// FreeCounter用のUAVのDescriptorRange
	D3D12_DESCRIPTOR_RANGE descriptorRangeFreeCounter[1] = {};
	descriptorRangeFreeCounter[0].BaseShaderRegister = 1;// u1
	descriptorRangeFreeCounter[0].NumDescriptors = 1; // 数は1つ
	descriptorRangeFreeCounter[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	descriptorRangeFreeCounter[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// RootParameterを作成
	D3D12_ROOT_PARAMETER rootParameters[4] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[0].DescriptorTable.pDescriptorRanges = descriptorRangeParticle;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeParticle);
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[1].Descriptor.ShaderRegister = 0;
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[2].Descriptor.ShaderRegister = 1;
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[3].DescriptorTable.pDescriptorRanges = descriptorRangeFreeCounter;
	rootParameters[3].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeFreeCounter);


	descriptionRootSignature.pParameters = rootParameters; // ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters); // 配列の長さ

	// Samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	// シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr <ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr <ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr))
	{
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリを元に生成
	hr = dxBasis_->GetDevice()->CreateRootSignature(0,
		signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&computeRootSignature));
	assert(SUCCEEDED(hr));
}

void ParticleManager::DrawSettingCompute()
{
	dxBasis_->GetCommandList()->SetComputeRootSignature(computeRootSignature.Get());
	dxBasis_->GetCommandList()->SetPipelineState(computePipelineState.Get());
}

void ParticleManager::CreateUav()
{
	// Particle用のUAVの生成
	particleUavIndex = srvManager_->Allocate();
	particleUavHandle.first = srvManager_->GetCPUDescriptorHandle(particleUavIndex);
	particleUavHandle.second = srvManager_->GetGPUDescriptorHandle(particleUavIndex);
	particleResource = dxBasis_->CreateOutputVertexBuffer(sizeof(ParticleCS) * 1024);
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = 1024;
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	uavDesc.Buffer.StructureByteStride = sizeof(ParticleCS);

	dxBasis_->GetDevice()->CreateUnorderedAccessView(particleResource.Get(), nullptr, &uavDesc, particleUavHandle.first);

	// Particle用のSRVを作成
	vertexIndex = srvManager_->Allocate();
	particleSrvHandle.first = srvManager_->GetCPUDescriptorHandle(vertexIndex);
	particleSrvHandle.second = srvManager_->GetGPUDescriptorHandle(vertexIndex);
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.NumElements = 1024;
	srvDesc.Buffer.StructureByteStride = sizeof(ParticleCS);
	dxBasis_->GetDevice()->CreateShaderResourceView(particleResource.Get(), &srvDesc, particleSrvHandle.first);

	// freeCounter用のUAVを生成
	// UAVの生成
	freeCounterUavIndex = srvManager_->Allocate();
	freeCounterUavHandle.first = srvManager_->GetCPUDescriptorHandle(freeCounterUavIndex);
	freeCounterUavHandle.second = srvManager_->GetGPUDescriptorHandle(freeCounterUavIndex);
	freeCounterResource = dxBasis_->CreateOutputVertexBuffer(sizeof(int32_t));
	D3D12_UNORDERED_ACCESS_VIEW_DESC freeCounterUavDesc{};
	freeCounterUavDesc.Format = DXGI_FORMAT_UNKNOWN;
	freeCounterUavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	freeCounterUavDesc.Buffer.FirstElement = 0;
	freeCounterUavDesc.Buffer.NumElements = 1;
	freeCounterUavDesc.Buffer.CounterOffsetInBytes = 0;
	freeCounterUavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	freeCounterUavDesc.Buffer.StructureByteStride = (sizeof(int32_t));

	dxBasis_->GetDevice()->CreateUnorderedAccessView(freeCounterResource.Get(), nullptr, &freeCounterUavDesc, freeCounterUavHandle.first);

	// CS用の設定
	srvManager_->PreDraw();
	DrawSettingCompute();
	dxBasis_->GetCommandList()->SetComputeRootDescriptorTable(0, particleUavHandle.second);
	dxBasis_->GetCommandList()->SetComputeRootDescriptorTable(3, freeCounterUavHandle.second);

	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = particleResource.Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	dxBasis_->GetCommandList()->ResourceBarrier(1, &barrier);
	dxBasis_->GetCommandList()->Dispatch(1, 1, 1);

	D3D12_RESOURCE_BARRIER barriers{};
	barriers.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barriers.Transition.pResource = particleResource.Get();
	barriers.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	barriers.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	barriers.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	dxBasis_->GetCommandList()->ResourceBarrier(1, &barriers);

}

void ParticleManager::CreateMaterialResource()
{
	// テクスチャファイルを読み込む
	TextureManager::GetInstance()->LoadTexture("resources/sprite/circle2.png");
	textureFilePath = "resources/sprite/circle2.png";
	materialData.textureFilePath = textureFilePath;
	materialData.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);

	// インスタンシング用リソースを生成
	instancingResource = dxBasis_->CreateBufferResources(sizeof(ParticleCS));
	// 書き込むためのアドレスを取得
	instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&particleData));

	// インスタンシング用にSRVを確保してインデックスを記録
	srvIndex = srvManager_->Allocate();

	// SRVの生成
	instancingSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	instancingSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	instancingSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	instancingSrvDesc.Buffer.FirstElement = 0;
	instancingSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	instancingSrvDesc.Buffer.NumElements = kMaxInstanceCount;
	instancingSrvDesc.Buffer.StructureByteStride = sizeof(ParticleForGPU);
	D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU = srvManager_->GetCPUDescriptorHandle(srvIndex);
	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU = srvManager_->GetGPUDescriptorHandle(srvIndex);
	srvManager_->CreateSRVforStructuredBuffer(srvIndex, instancingResource.Get(), instancingSrvDesc.Format, kMaxInstanceCount, sizeof(ParticleCS));

}

void ParticleManager::CreatePerViewResource()
{
	perViewResource = dxBasis_->CreateBufferResources(sizeof(PerView));
	perViewResource->Map(0, nullptr, reinterpret_cast<void**>(&perView));
}

void ParticleManager::CreateEmitterResource()
{
	emitterResource = dxBasis_->CreateBufferResources(sizeof(EmitterSphere));
	emitterResource->Map(0, nullptr, reinterpret_cast<void**>(&emitterSphere));
}

void ParticleManager::CreatePerFrameResource()
{
	perFrameResource = dxBasis_->CreateBufferResources(sizeof(PerFrame));
	perFrameResource->Map(0, nullptr, reinterpret_cast<void**>(&perFrame));
}


ParticleManager::Particle ParticleManager::MakeNewNormalParticle(const Vector3& translate, const Vector3& scale, const Vector3& rotate,
	const Vector3& velocity, const Vector4& color, const float lifeTime, const float currentTime)
{
	// ランダムエンジンの初期化
	std::random_device seedGenerator;
	std::mt19937 randomEngine(seedGenerator());
	std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
	randomTranslate = { distribution(randomEngine),distribution(randomEngine) ,distribution(randomEngine) };
	std::uniform_real_distribution<float> distTime(1.0f, 3.0f);

	particle.transform.scale = scale;
	particle.transform.rotate = rotate;
	particle.transform.translate = translate;
	particle.velocity = { distribution(randomEngine),distribution(randomEngine),distribution(randomEngine) };
	particle.color = { distribution(randomEngine),distribution(randomEngine),distribution(randomEngine) ,1.0f };
	particle.lifeTime = distTime(randomEngine);
	particle.currentTime = currentTime;

	return particle;
}

ParticleManager::Particle ParticleManager::MakeNewHitEffectParticle(const Vector3& translate, const Vector3& scale, const Vector3& rotate,
	const Vector3& velocity, const Vector4& color, const float lifeTime, const float currentTime)
{
	// ランダムエンジンの初期化
	std::random_device seedGenerator;
	std::mt19937 randomEngine(seedGenerator());
	std::uniform_real_distribution<float> distRotate(-std::numbers::pi_v<float>, std::numbers::pi_v<float>);
	std::uniform_real_distribution<float> distScale(0.4f, 1.5f);

	particle.transform.scale = { scale.x,distScale(randomEngine),scale.z };;
	particle.transform.rotate = { rotate.x,rotate.y,distRotate(randomEngine) };
	particle.transform.translate = translate;
	particle.velocity = velocity;
	particle.color = color;
	particle.lifeTime = lifeTime;
	particle.currentTime = currentTime;
	return particle;
}

ParticleManager::Particle ParticleManager::MakeNewRingParticle(const Vector3& translate, const Vector3& scale, const Vector3& rotate,
	const Vector3& velocity, const Vector4& color, const float lifeTime, const float currentTime)
{
	particle.transform.scale = scale;
	particle.transform.rotate = rotate;
	particle.transform.translate = translate;
	particle.velocity = velocity;
	particle.color = color;
	particle.lifeTime = lifeTime;
	particle.currentTime = currentTime;
	return particle;
}

ParticleManager::Particle ParticleManager::MakeNewCylinderParticle(const Vector3& translate, const Vector3& scale, const Vector3& rotate,
	const Vector3& velocity, const Vector4& color, const float lifeTime, const float currentTime)
{
	particle.transform.scale = scale;
	particle.transform.rotate = rotate;
	particle.transform.translate = translate;
	particle.velocity = velocity;
	particle.color = color;
	particle.lifeTime = lifeTime;
	particle.currentTime = currentTime;
	return particle;
}


bool ParticleManager::IsCollision(const AABB& aabb, const Vector3& point) {
	if (aabb.min.x <= point.x && aabb.max.x >= point.x &&
		aabb.min.y <= point.y && aabb.max.y >= point.y &&
		aabb.min.z <= point.z && aabb.max.z >= point.z) {
		return true;
	}

	return false;
}

void ParticleManager::CreateParticleGroup(const std::string name, const std::string textureFilePath, ParticleEmitter::Type type)
{
	// 登録済みの名前かチェック
	const bool alreadyExists =
		(particleGroups.find(name) != particleGroups.end());
	assert(!alreadyExists);

	// 新たなパーティクルグループを作成
	ParticleGroup newGrounp{};
	particleGroups.emplace(name, std::move(newGrounp));

	// 追加したマテリアルデータの参照を取得する
	ParticleGroup& particleData = particleGroups.at(name);

	// テクスチャファイルを読み込む
	particleData.materialData.textureFilePath = textureFilePath;
	particleData.materialData.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);

	// インスタンシング用リソースを生成
	particleData.instancingResource = dxBasis_->CreateBufferResources(sizeof(ParticleForGPU) * kMaxInstanceCount);
	// 書き込むためのアドレスを取得
	particleData.instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&particleData.instancingData));
	// 単位行列を書き込んでおく
	for (uint32_t index = 0; index < kMaxInstanceCount; ++index)
	{
		particleData.instancingData[index].WVP = MakeIdentity4x4();
		particleData.instancingData[index].World = MakeIdentity4x4();
		particleData.instancingData[index].Color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	// パーティクルグループのタイプを記録
	particleData.type = type;

	// インスタンシング用にSRVを確保してインデックスを記録
	particleData.srvIndex = srvManager_->Allocate();

	// SRVの生成
	instancingSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	instancingSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	instancingSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	instancingSrvDesc.Buffer.FirstElement = 0;
	instancingSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	instancingSrvDesc.Buffer.NumElements = kMaxInstanceCount;
	instancingSrvDesc.Buffer.StructureByteStride = sizeof(ParticleForGPU);
	D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU = srvManager_->GetCPUDescriptorHandle(particleData.srvIndex);
	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU = srvManager_->GetGPUDescriptorHandle(particleData.srvIndex);
	srvManager_->CreateSRVforStructuredBuffer(particleData.srvIndex, particleData.instancingResource.Get(), instancingSrvDesc.Format, kMaxInstanceCount, sizeof(ParticleForGPU));
}

void ParticleManager::Update()
{
#ifdef USE_IMGUI

	ImGui::Begin("Billboard");
	ImGui::Checkbox("Billboard", &isBillboard);
	ImGui::End();

#endif


	// ビルボードの計算処理
	Matrix4x4 cameraMatrix = camera->GetWorldMatrix();
	Matrix4x4 billboardMatrix = MakeIdentity4x4();

	if (isBillboard)
	{
		Matrix4x4 backToFrontMatrix = MakeRotateYMatrix(std::numbers::pi_v<float>);
		billboardMatrix = Multiply(backToFrontMatrix, cameraMatrix);
		billboardMatrix.m[3][0] = 0.0f;
		billboardMatrix.m[3][1] = 0.0f;
		billboardMatrix.m[3][2] = 0.0f;
	}

	emitterSphere->frequencyTime += kDeltaTime;

	if (emitterSphere->frequency <= emitterSphere->frequencyTime)
	{
		emitterSphere->frequencyTime = emitterSphere->frequency;
		emitterSphere->emit = 1;
	}
	else
	{
		emitterSphere->emit = 0;
	}

	perView->billboardMatrix = billboardMatrix;
	perView->viewProjection = camera->GetViewProjectionMatrix();

	perFrame->time += kDeltaTime;
	perFrame->deltaTime = kDeltaTime;

	//	// 全てのパーティクルグループ
	//	for (auto& [name, group] : particleGroups)
	//	{
	//		group.instanceCount = 0;
	//		// 各グループのリスト
	//		for (auto it = group.particles.begin(); it != group.particles.end();)
	//		{
	//			if (it->lifeTime <= it->currentTime)
	//			{
	//				// 生存時間を過ぎていたら表示しない
	//				it = group.particles.erase(it);
	//				continue;
	//			}
	//
	//			// Fieldの範囲内のParticleにはAccelerationを適用
	//			if (IsCollision(accelerationField.area, it->transform.translate))
	//			{
	//				Vector3 accelDelta =
	//				{
	//				   accelerationField.acceleration.x * kDeltaTime,
	//				   accelerationField.acceleration.y * kDeltaTime,
	//				   accelerationField.acceleration.z * kDeltaTime
	//				};
	//				it->velocity = Vector3Add(it->velocity, accelDelta);
	//			}
	//
	//			it->transform.translate = Vector3Add(it->transform.translate, FloatMultiply(it->velocity, kDeltaTime));
	//			it->currentTime += kDeltaTime;
	//			float alpha = 1.0f - (it->currentTime / it->lifeTime);
	//			it->color.w = alpha;
	//
	//			Matrix4x4 scaleMatrix = MakeScaleMatrix(it->transform.scale);
	//			Matrix4x4 translateMatrix = MakeTranslateMatrix(it->transform.translate);
	//			Matrix4x4 worldMatrix;
	//
	//			if (isBillboard)
	//			{
	//				worldMatrix = Multiply(scaleMatrix, billboardMatrix);
	//				worldMatrix = Multiply(worldMatrix, translateMatrix);
	//			}
	//			else
	//			{
	//				worldMatrix = MakeAffineMatrix(it->transform.scale, it->transform.rotate, it->transform.translate);
	//			}
	//
	//			Matrix4x4 viewProjectionMatrix = camera->GetViewProjectionMatrix();
	//			Matrix4x4 wvp = Multiply(worldMatrix, viewProjectionMatrix);
	//
	//			// インスタンシング用データ1個分を書き込み
	//			if (group.instanceCount < kMaxInstanceCount) {
	//				group.instancingData[group.instanceCount].World = worldMatrix;
	//				group.instancingData[group.instanceCount].WVP = wvp;
	//				group.instancingData[group.instanceCount].Color = it->color;
	//				group.instancingData[group.instanceCount].Color.w = alpha;
	//
	//				group.instanceCount++;
	//			}
	//
	//			
	//
	//#ifdef USE_IMGUI
	//
	//			ImGui::Begin("Particle Manager");
	//			ImGui::DragFloat3("Position", &it->transform.translate.x, 0.1f);
	//			
	//			ImGui::End();
	//
	//#endif
	//
	//
	//
	//
	//
	//			++it;
	//
	//		}
	//
	//	}

}

void ParticleManager::Draw()
{
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = emitterResource.Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	// emitterデータを転送
	DrawSettingCompute();
	dxBasis_->GetCommandList()->SetComputeRootDescriptorTable(0, particleUavHandle.second);
	dxBasis_->GetCommandList()->SetComputeRootConstantBufferView(1, emitterResource->GetGPUVirtualAddress());
	dxBasis_->GetCommandList()->SetComputeRootConstantBufferView(2, perFrameResource->GetGPUVirtualAddress());
	dxBasis_->GetCommandList()->SetComputeRootDescriptorTable(3, freeCounterUavHandle.second);
	dxBasis_->GetCommandList()->SetPipelineState(computePipelineStateEmit.Get());
	dxBasis_->GetCommandList()->Dispatch(1, 1, 1);

	D3D12_RESOURCE_BARRIER barriers{};
	barriers.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barriers.Transition.pResource = emitterResource.Get();
	barriers.Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
	barriers.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	barriers.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	dxBasis_->GetCommandList()->ResourceBarrier(1, &barriers);



	// RootSignatureを設定
	dxBasis_->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
	// PSOを設定
	dxBasis_->GetCommandList()->SetPipelineState(graphicPipelineState.Get());
	// 形状を設定
	dxBasis_->GetCommandList()->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	// テクスチャのSRVのDescriptorTableを設定
	dxBasis_->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSRVHandleGPU(materialData.textureFilePath));
	// SRVのDescriptorTableの先頭を設定
	dxBasis_->GetCommandList()->SetGraphicsRootDescriptorTable(1, srvManager_->GetGPUDescriptorHandle(srvIndex));

	// Particleデータを格納
	dxBasis_->GetCommandList()->SetGraphicsRootDescriptorTable(1, particleSrvHandle.second);
	// PerViewデータを転送
	dxBasis_->GetCommandList()->SetGraphicsRootConstantBufferView(3, perViewResource->GetGPUVirtualAddress());

	// VBVを設定
	dxBasis_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
	// 描画
	dxBasis_->GetCommandList()->DrawInstanced(6, 1024, 0, 0);


	//// グループごとに描画
	//for (auto& [name, group] : particleGroups)
	//{
	//	if (group.instanceCount == 0)
	//	{
	//		// 一つもなかったら描画をスキップ
	//		continue;
	//	}

	//	switch (group.type)
	//	{
	//	case ParticleEmitter::Type::kNormal:
	//		// VBVを設定
	//		dxBasis_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
	//		// 描画
	//		dxBasis_->GetCommandList()->DrawInstanced(6, group.instanceCount, 0, 0);
	//		break;

	//	case ParticleEmitter::Type::kHitEffect:
	//		// VBVを設定
	//		dxBasis_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
	//		// 描画
	//		dxBasis_->GetCommandList()->DrawInstanced(static_cast<UINT>(modelData.vertices.size()), group.instanceCount, 0, 0);
	//		break;

	//	case ParticleEmitter::Type::kRing:
	//		// VBVを設定
	//		dxBasis_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewRing);
	//		// 描画
	//		dxBasis_->GetCommandList()->DrawInstanced(static_cast<UINT>(modelDataRing.vertices.size()), group.instanceCount, 0, 0);
	//		break;

	//	case ParticleEmitter::Type::kCylinder:
	//		// VBVを設定
	//		dxBasis_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewCylinder);
	//		// 描画
	//		dxBasis_->GetCommandList()->DrawInstanced(static_cast<UINT>(modelDataCylinder.vertices.size()), group.instanceCount, 0, 0);
	//		break;
	//	}
	//	
	//}
}

void ParticleManager::Emit(const std::string name, const Vector3& translate, const Vector3& scale, const Vector3& rotate,
	const Vector3& velocity, const Vector4& color, const float lifeTime, const float currentTime, uint32_t count)
{
	// 登録済みかチェック
	auto it = particleGroups.find(name);
	assert(it != particleGroups.end() && "Particle group not found.");
	ParticleGroup& group = it->second;

	// パーティクルグループを作成して登録
	for (uint32_t i = 0; i < count; ++i) {
		if (group.particles.size() >= kMaxInstanceCount) {
			break; // 上限以上は積まない
		}

		switch (group.type)
		{
		case ParticleEmitter::Type::kNormal:
			group.particles.push_back(MakeNewNormalParticle(translate, scale, rotate, velocity, color, lifeTime, currentTime));

			break;

		case ParticleEmitter::Type::kHitEffect:
			group.particles.push_back(MakeNewHitEffectParticle(translate, scale, rotate, velocity, color, lifeTime, currentTime));


			break;
		case ParticleEmitter::Type::kRing:
			group.particles.push_back(MakeNewRingParticle(translate, scale, rotate, velocity, color, lifeTime, currentTime));


			break;

		case ParticleEmitter::Type::kCylinder:
			group.particles.push_back(MakeNewCylinderParticle(translate, scale, rotate, velocity, color, lifeTime, currentTime));
			break;
		}

	}
}