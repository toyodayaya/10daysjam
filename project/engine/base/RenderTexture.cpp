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
	vertexShaderBlob = dxBasis_->CompileShader(L"resources/shaders/postEffect/FullScreen.VS.hlsl",
		L"vs_6_0");
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr <IDxcBlob> pixelShaderBlob;
	pixelShaderBlob = dxBasis_->CompileShader(L"resources/shaders/postEffect/FullScreen.PS.hlsl",
		L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	// Depthの機能を有効化する
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = false;

	// PSOを生成する
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

	// 他のシェーダーもコンパイルしておく
	GeneratePostEffect();
}

void RenderTexture::GeneratePostEffect()
{
	// 各シェーダーをコンパイル

	// GrayScale
	GenerateGrayScale();

	// SepiaScale
	GenerateSepia();

	// Vignetting
	GenerateVignetting();
	
	// BoxFilter
	GenerateBoxFilter();
	
	// GaussianFilter
	GenerateGaussianFilter();

	// OutLine
	GenerateOutline();
	
	// radialBlur
	GenerateRadialBlur();

	// Dissolve
	GenerateDissolve();
	
	// Random
	GenerateRandom();
	
}

void RenderTexture::GenerateGrayScale()
{
	// 設定をコピー
	grayscaleGPSD_ = graphicPipelineStateDesc;

	// PSをコンパイル
	Microsoft::WRL::ComPtr <IDxcBlob> pixelShaderBlobGrayscale;
	pixelShaderBlobGrayscale = dxBasis_->CompileShader(L"resources/shaders/postEffect/GrayScale.PS.hlsl",
		L"ps_6_0");
	assert(pixelShaderBlobGrayscale != nullptr);
	grayscaleGPSD_.PS =
	{
		pixelShaderBlobGrayscale->GetBufferPointer(),
		pixelShaderBlobGrayscale->GetBufferSize()
	};
	// 生成
	HRESULT hr = dxBasis_->GetDevice()->CreateGraphicsPipelineState(&grayscaleGPSD_,
		IID_PPV_ARGS(&grayscaleGPS_));
	assert(SUCCEEDED(hr));

}

void RenderTexture::GenerateSepia()
{
	// 設定をコピー
	sepiascaleGPSD_ = graphicPipelineStateDesc;

	// PSをコンパイル
	Microsoft::WRL::ComPtr <IDxcBlob> pixelShaderBlobSepia;
	pixelShaderBlobSepia = dxBasis_->CompileShader(L"resources/shaders/postEffect/SepiaScale.PS.hlsl",
		L"ps_6_0");
	assert(pixelShaderBlobSepia != nullptr);
	sepiascaleGPSD_.PS =
	{
		pixelShaderBlobSepia->GetBufferPointer(),
		pixelShaderBlobSepia->GetBufferSize()
	};
	// 生成
	HRESULT hr = dxBasis_->GetDevice()->CreateGraphicsPipelineState(&sepiascaleGPSD_,
		IID_PPV_ARGS(&sepiascaleGPS_));
	assert(SUCCEEDED(hr));
}

void RenderTexture::GenerateVignetting()
{
	// 設定をコピー
	vignettingGPSD_ = graphicPipelineStateDesc;

	// PSをコンパイル
	Microsoft::WRL::ComPtr <IDxcBlob> pixelShaderBlobVignetting;
	pixelShaderBlobVignetting = dxBasis_->CompileShader(L"resources/shaders/postEffect/Vignette.PS.hlsl",
		L"ps_6_0");
	assert(pixelShaderBlobVignetting != nullptr);
	vignettingGPSD_.PS =
	{
		pixelShaderBlobVignetting->GetBufferPointer(),
		pixelShaderBlobVignetting->GetBufferSize()
	};
	// 生成
	HRESULT hr = dxBasis_->GetDevice()->CreateGraphicsPipelineState(&vignettingGPSD_,
		IID_PPV_ARGS(&vignettingGPS_));
	assert(SUCCEEDED(hr));


}

void RenderTexture::GenerateBoxFilter()
{
	// 設定をコピー
	boxFilterGPSD_ = graphicPipelineStateDesc;

	// PSをコンパイル
	Microsoft::WRL::ComPtr <IDxcBlob> pixelShaderBlobBoxFilter;
	pixelShaderBlobBoxFilter = dxBasis_->CompileShader(L"resources/shaders/postEffect/BoxFilter5x5.PS.hlsl",
		L"ps_6_0");
	assert(pixelShaderBlobBoxFilter != nullptr);
	boxFilterGPSD_.PS =
	{
		pixelShaderBlobBoxFilter->GetBufferPointer(),
		pixelShaderBlobBoxFilter->GetBufferSize()
	};
	// 生成
	HRESULT hr = dxBasis_->GetDevice()->CreateGraphicsPipelineState(&boxFilterGPSD_,
		IID_PPV_ARGS(&boxFilterGPS_));
	assert(SUCCEEDED(hr));

}

void RenderTexture::GenerateGaussianFilter()
{
	// 設定をコピー
	gaussianFilterGPSD_ = graphicPipelineStateDesc;

	// PSをコンパイル
	Microsoft::WRL::ComPtr <IDxcBlob> pixelShaderBlobGaussianFilter;
	pixelShaderBlobGaussianFilter = dxBasis_->CompileShader(L"resources/shaders/postEffect/GaussianFilter.PS.hlsl",
		L"ps_6_0");
	assert(pixelShaderBlobGaussianFilter != nullptr);
	gaussianFilterGPSD_.PS =
	{
		pixelShaderBlobGaussianFilter->GetBufferPointer(),
		pixelShaderBlobGaussianFilter->GetBufferSize()
	};
	// 生成
	HRESULT hr = dxBasis_->GetDevice()->CreateGraphicsPipelineState(&gaussianFilterGPSD_,
		IID_PPV_ARGS(&gaussianFilterGPS_));
	assert(SUCCEEDED(hr));
}

void RenderTexture::GenerateOutline()
{
	// 設定をコピー
	outlineGPSD_ = graphicPipelineStateDesc;

	// PSをコンパイル
	Microsoft::WRL::ComPtr <IDxcBlob> pixelShaderBlobOutLine;
	pixelShaderBlobOutLine = dxBasis_->CompileShader(L"resources/shaders/postEffect/DepthBasedoutline.PS.hlsl",
		L"ps_6_0");
	assert(pixelShaderBlobOutLine != nullptr);
	outlineGPSD_.PS =
	{
		pixelShaderBlobOutLine->GetBufferPointer(),
		pixelShaderBlobOutLine->GetBufferSize()
	};
	// 生成
	HRESULT hr = dxBasis_->GetDevice()->CreateGraphicsPipelineState(&outlineGPSD_,
		IID_PPV_ARGS(&outlineGPS_));
	assert(SUCCEEDED(hr));

}

void RenderTexture::GenerateRadialBlur()
{
	// 設定をコピー
	radialBlurGPSD_ = graphicPipelineStateDesc;

	// PSをコンパイル
	Microsoft::WRL::ComPtr <IDxcBlob> pixelShaderBlobRadialBlur;
	pixelShaderBlobRadialBlur = dxBasis_->CompileShader(L"resources/shaders/postEffect/RadialBlur.PS.hlsl",
		L"ps_6_0");
	assert(pixelShaderBlobRadialBlur != nullptr);
	radialBlurGPSD_.PS =
	{
		pixelShaderBlobRadialBlur->GetBufferPointer(),
		pixelShaderBlobRadialBlur->GetBufferSize()
	};
	// 生成
	HRESULT hr = dxBasis_->GetDevice()->CreateGraphicsPipelineState(&radialBlurGPSD_,
		IID_PPV_ARGS(&radialBlurGPS_));
	assert(SUCCEEDED(hr));
}

void RenderTexture::GenerateDissolve()
{
	// 設定をコピー
	dissolveGPSD_ = graphicPipelineStateDesc;

	// PSをコンパイル
	Microsoft::WRL::ComPtr <IDxcBlob> pixelShaderBlobDissolve;
	pixelShaderBlobDissolve = dxBasis_->CompileShader(L"resources/shaders/postEffect/Dissolve.PS.hlsl",
		L"ps_6_0");
	assert(pixelShaderBlobDissolve != nullptr);
	dissolveGPSD_.PS =
	{
		pixelShaderBlobDissolve->GetBufferPointer(),
		pixelShaderBlobDissolve->GetBufferSize()
	};
	// 生成
	HRESULT hr = dxBasis_->GetDevice()->CreateGraphicsPipelineState(&dissolveGPSD_,
		IID_PPV_ARGS(&dissolveGPS_));
	assert(SUCCEEDED(hr));

}

void RenderTexture::GenerateRandom()
{
	// 設定をコピー
	randomGPSD_ = graphicPipelineStateDesc;

	// PSをコンパイル
	Microsoft::WRL::ComPtr <IDxcBlob> pixelShaderBlobRandom;
	pixelShaderBlobRandom = dxBasis_->CompileShader(L"resources/shaders/postEffect/Random.PS.hlsl",
		L"ps_6_0");
	assert(pixelShaderBlobRandom != nullptr);
	randomGPSD_.PS =
	{
		pixelShaderBlobRandom->GetBufferPointer(),
		pixelShaderBlobRandom->GetBufferSize()
	};
	// 生成
	HRESULT hr = dxBasis_->GetDevice()->CreateGraphicsPipelineState(&randomGPSD_,
		IID_PPV_ARGS(&randomGPS_));
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
	// RootSignatureを設定
	dxBasis_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());

	// タイプに合わせて使うPSOを変更
	DrawSettingPSO();

	// 形状を設定
	dxBasis_->GetCommandList()->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// SRVを設定
	dxBasis_->GetCommandList()->SetGraphicsRootDescriptorTable(2, handle_);
	// 描画
	dxBasis_->GetCommandList()->DrawInstanced(3, 1, 0, 0);
}

void RenderTexture::DrawSettingPSO()
{
	// タイプに合わせて使うPSOを変更
	switch (type_)
	{
	case kNormal:
		// 通常のシェーダー
		dxBasis_->GetCommandList()->SetPipelineState(graphicPipelineState_.Get());
		break;

	case kGrayScale:
		// GrayScaleのシェーダー
		dxBasis_->GetCommandList()->SetPipelineState(grayscaleGPS_.Get());
		break;

	case kSepiaScale:
		// Sepiaのシェーダー
		dxBasis_->GetCommandList()->SetPipelineState(sepiascaleGPS_.Get());
		break;

	case kVignetting:
		// Vignettingのシェーダー
		dxBasis_->GetCommandList()->SetPipelineState(vignettingGPS_.Get());
		break;

	case kBoxFilter:
		// BoxFilterのシェーダー
		dxBasis_->GetCommandList()->SetPipelineState(boxFilterGPS_.Get());
		break;

	case kGaussianFilter:
		// GaussianFilterのシェーダー
		dxBasis_->GetCommandList()->SetPipelineState(gaussianFilterGPS_.Get());
		break;

	case kOutline:
		// OutLineのシェーダー
		dxBasis_->GetCommandList()->SetPipelineState(outlineGPS_.Get());
		// SRVを設定
		dxBasis_->GetCommandList()->SetGraphicsRootDescriptorTable(3, depthHandle_);
		// CBufferの場所を設定
		projectionInverseData_->projectionInverse = Inverse(defaultCamera_->GetProjectionMatrix());
		dxBasis_->GetCommandList()->SetGraphicsRootConstantBufferView(0, projecttionInverseResource_->GetGPUVirtualAddress());
		break;

	case kRadialBlur:
		// RadialBlurのシェーダー
		dxBasis_->GetCommandList()->SetPipelineState(radialBlurGPS_.Get());
		break;

	case kDissolve:
		// Dissolveのシェーダー
		dxBasis_->GetCommandList()->SetPipelineState(dissolveGPS_.Get());
		dxBasis_->GetCommandList()->SetGraphicsRootDescriptorTable(3, dissolveHandle_);
		break;

	case kRandom:
		// Randomのシェーダー
		dxBasis_->GetCommandList()->SetPipelineState(randomGPS_.Get());
		// CBufferの場所を設定
		timeData_->time += kDeltaTime;
		dxBasis_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialTimeResource_->GetGPUVirtualAddress());
		break;
	}
}

void RenderTexture::Finalize()
{
	instance.reset();
}
