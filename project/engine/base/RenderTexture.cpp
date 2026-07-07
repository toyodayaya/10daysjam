#include "RenderTexture.h"
#include "Logger.h"
#include "MathManager.h"
#include "TextureManager.h"

using namespace MathManager;

std::unique_ptr<RenderTexture> RenderTexture::instance = nullptr;

RenderTexture* RenderTexture::GetInstance()
{
	if (instance == nullptr)
	{
		instance = std::make_unique<RenderTexture>();
	}

	return instance.get();
}

void RenderTexture::Initialize(DirectXBasis* directXBasis,SrvManager* srvManager)
{
	// 引数で受け取ってメンバ変数として記録する
	dxBasis_ = directXBasis;
	srvManager_ = srvManager;

	// グラフィックスパイプラインの生成
	GenerateGraphicsPipeline();

	// アウトライン用の逆行列を作成
	CreateProjectionInverse();

	// 経過時間を作成
	CreateMaterialTime();

	// 描画用テクスチャを読み込む
	filePath_ = "resources/sprite/uvChecker.png";
	dissolveFilePath_ = "resources/sprite/noise0.png";
	TextureManager::GetInstance()->LoadRenderTexture(filePath_);
	TextureManager::GetInstance()->LoadTexture(dissolveFilePath_);
	texture_ = TextureManager::GetInstance()->GetRenderTextureData(filePath_);
	// RenderTextureの描画準備
	handle_ = TextureManager::GetInstance()->GetRenderSRVHandleGPU(filePath_);
	depthHandle_ = TextureManager::GetInstance()->GetDepthSRVHandle(filePath_);
	dissolveHandle_ = TextureManager::GetInstance()->GetSRVHandleGPU(dissolveFilePath_);
}

void RenderTexture::CreateRootSignature()
{
	// RootSignatureを作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE rangeDepth[1] = {};
	rangeDepth[0].BaseShaderRegister = 1; // t1
	rangeDepth[0].NumDescriptors = 1;     // 1枚だけ
	rangeDepth[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	rangeDepth[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// RootParameterを作成
	D3D12_ROOT_PARAMETER rootParameters[4] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	rootParameters[0].Descriptor.ShaderRegister = 0; // レジスタ番号0とバインド
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // VertexShaderで使う
	rootParameters[1].Descriptor.ShaderRegister = 0; // レジスタ番号0とバインド
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[3].DescriptorTable.pDescriptorRanges = rangeDepth;
	rootParameters[3].DescriptorTable.NumDescriptorRanges = _countof(rangeDepth);
	descriptionRootSignature.pParameters = rootParameters; // ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters); // 配列の長さ


	// Samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[2] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	staticSamplers[1].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	staticSamplers[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[1].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[1].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[1].ShaderRegister = 1;
	staticSamplers[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	// シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr <ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr <ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr))
	{
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリを元に生成
	hr = dxBasis_->GetDevice()->CreateRootSignature(0,
		signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));
}

void RenderTexture::GenerateGraphicsPipeline()
{
	// ルートシグネチャーの作成
	CreateRootSignature();

	// InputLayout
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = nullptr;
	inputLayoutDesc.NumElements = 0;

	// 全ての色要素を書き込む
	blendDesc_.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc_.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc_.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc_.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

	// 三角形の中を塗りつぶす
	// RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	// カリングしない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;

	// ShaderをCompileする
	Microsoft::WRL::ComPtr <IDxcBlob> vertexShaderBlob;
	vertexShaderBlob = dxBasis_->CompileShader(L"resources/shaders/FullScreen.VS.hlsl",
		L"vs_6_0");
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr <IDxcBlob> pixelShaderBlob;
	pixelShaderBlob = dxBasis_->CompileShader(L"resources/shaders/FullScreen.PS.hlsl",
		L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	// Depthの機能を有効化する
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = false;

	// PSOを生成する
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicPipelineStateDesc{};
	graphicPipelineStateDesc.BlendState = blendDesc_;
	graphicPipelineStateDesc.pRootSignature = rootSignature_.Get();
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
		IID_PPV_ARGS(&graphicPipelineState_));
	assert(SUCCEEDED(hr));
}


void RenderTexture::CreateProjectionInverse()
{
	// WVP用のリソースを作る
	projecttionInverseResource_ = dxBasis_->CreateBufferResources(sizeof(Material));
	// データを書き込む
	// 書き込むためのアドレスを取得
	projecttionInverseResource_->Map(0, nullptr, reinterpret_cast<void**>(&projectionInverseData_));
	// 単位行列を書き込んでおく
	projectionInverseData_->projectionInverse = Inverse(defaultCamera_->GetProjectionMatrix());
}

void RenderTexture::CreateMaterialTime()
{
	// 経過時間用のリソースを作る
	materialTimeResource_ = dxBasis_->CreateBufferResources(sizeof(MaterialTime));
	// データを書き込む
	// 書き込むためのアドレスを取得
	materialTimeResource_->Map(0, nullptr, reinterpret_cast<void**>(&timeData_));
	//	時間を加算
	timeData_->time += kDeltaTime;
}

void RenderTexture::DrawSettingCommon()
{
	projectionInverseData_->projectionInverse = Inverse(defaultCamera_->GetProjectionMatrix());
	timeData_->time += kDeltaTime;

	// RootSignatureを設定
	dxBasis_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
	// PSOを設定
	dxBasis_->GetCommandList()->SetPipelineState(graphicPipelineState_.Get());
	// 形状を設定
	dxBasis_->GetCommandList()->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// CBufferの場所を設定
	dxBasis_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialTimeResource_->GetGPUVirtualAddress());
	// SRVを設定
	dxBasis_->GetCommandList()->SetGraphicsRootDescriptorTable(2, handle_);
	dxBasis_->GetCommandList()->SetGraphicsRootDescriptorTable(3, dissolveHandle_);
	// 描画
	dxBasis_->GetCommandList()->DrawInstanced(3, 1, 0, 0);
}

void RenderTexture::Finalize()
{
	instance.reset();
}
