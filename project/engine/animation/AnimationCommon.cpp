#include "AnimationCommon.h"
#include "Logger.h"
using namespace Logger;

std::unique_ptr<AnimationCommon> AnimationCommon::instance = nullptr;

AnimationCommon* AnimationCommon::GetInstance()
{
	if (instance == nullptr)
	{
		instance = std::make_unique<AnimationCommon>();
	}

	return instance.get();
}

void AnimationCommon::Finalize()
{
	instance.reset();
}


void AnimationCommon::Initialize(DirectXBasis* directXBasis, SrvManager* srvManager)
{
	// 引数で受け取ってメンバ変数として記録する
	dxBasis_ = directXBasis;
	srvManager_ = srvManager;

	// CSのパイプラインの生成
	GenerateCSPipelineState();
}

void AnimationCommon::GenerateCSPipelineState()
{
	// ルートシグネチャーを作成
	CreateCSRootSignature();

	// shaderをcompileする
	computeShaderBlob = dxBasis_->CompileShader(L"resources/shaders/object3d/Skinning.CS.hlsl",
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
}

void AnimationCommon::CreateCSRootSignature()
{
	// RootSignatureを作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;// t0
	descriptorRange[0].NumDescriptors = 1; // 数は1つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	D3D12_DESCRIPTOR_RANGE descriptorRangeInput[1] = {};
	descriptorRangeInput[0].BaseShaderRegister = 1;// t1
	descriptorRangeInput[0].NumDescriptors = 1; // 数は1つ
	descriptorRangeInput[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRangeInput[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	D3D12_DESCRIPTOR_RANGE descriptorRangeInfluence[1] = {};
	descriptorRangeInfluence[0].BaseShaderRegister = 2;// t2
	descriptorRangeInfluence[0].NumDescriptors = 1; // 数は1つ
	descriptorRangeInfluence[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRangeInfluence[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	D3D12_DESCRIPTOR_RANGE descriptorRangeOutput[1] = {};
	descriptorRangeOutput[0].BaseShaderRegister = 0;// u0
	descriptorRangeOutput[0].NumDescriptors = 1; // 数は1つ
	descriptorRangeOutput[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	descriptorRangeOutput[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// RootParameterを作成
	D3D12_ROOT_PARAMETER rootParameters[5] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // CSで使う
	rootParameters[0].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // CSで使う
	rootParameters[1].DescriptorTable.pDescriptorRanges = descriptorRangeInput;
	rootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeInput);

	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRangeInfluence;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeInfluence);

	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[3].DescriptorTable.pDescriptorRanges = descriptorRangeOutput;
	rootParameters[3].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeOutput);

	rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[4].Descriptor.ShaderRegister = 0; // レジスタ番号0とバインド
	
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

void AnimationCommon::DrawSettingCompute()
{
	dxBasis_->GetCommandList()->SetComputeRootSignature(computeRootSignature.Get());
	dxBasis_->GetCommandList()->SetPipelineState(computePipelineState.Get());
}
