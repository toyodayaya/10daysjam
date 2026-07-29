#include "TextureManager.h"
#include "DirectXBasis.h"
#include "SrvManager.h"
#include "StringUtility.h"
#include <algorithm>

using namespace StringUtility;


std::unique_ptr<TextureManager> TextureManager::instance = nullptr;

// ImGuiで０番を使用するため1番から使用
uint32_t TextureManager::kSRVIndexTop = 1;

TextureManager* TextureManager::GetInstance()
{
	if (instance == nullptr)
	{
		instance = std::make_unique<TextureManager>(ConstructorKey());
	}

	return instance.get();
}

void TextureManager::Finalize()
{
	instance.reset();
}

void TextureManager::Initialize(DirectXBasis* dxBasis, SrvManager* srvManager)
{
	// SRVの数と同数
	textureDatas.reserve(SrvManager::kMaxSRVCount);

	// 引数で受け取ってメンバ変数として記録する
	this->dxBasis_ = dxBasis;
	this->srvManager_ = srvManager;
}

void TextureManager::LoadTexture(const std::string& filePath)
{
	// 読み込み済みテクスチャを検索
	if (textureDatas.contains(filePath))
	{
		return;
	}

	// テクスチャ枚数上限チェック
	assert(srvManager_->CanAllocate());

	// テクスチャファイルを読み込んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);

	// DDSかどうか判定
	HRESULT hr;
	if (filePathW.ends_with(L".dds"))
	{
		// DDSを読み込む
		hr = DirectX::LoadFromDDSFile(filePathW.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
	}
	else
	{
		hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	}

	assert(SUCCEEDED(hr));

	// ミニマップの作成
	DirectX::ScratchImage mipImages{};

	// 圧縮フォーマットかどうか判定
	if (DirectX::IsCompressed(image.GetMetadata().format))
	{
		// 圧縮フォーマットならそのまま使う
		mipImages = std::move(image);
	}
	else
	{
		hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
		assert(SUCCEEDED(hr));
	}
	
	// 追加したテクスチャデータの参照を取得する
	TextureData& textureData = textureDatas[filePath];

	// 追加したテクスチャデータに書き込む
	textureData.metaData = mipImages.GetMetadata();
	textureData.resource = dxBasis_->CreateTextureResource(textureData.metaData);
	
	// テクスチャデータの要素数番号をSRVのインデックスとする
	textureData.srvIndex = srvManager_->Allocate();
	textureData.srvHandleCPU = srvManager_->GetCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = srvManager_->GetGPUDescriptorHandle(textureData.srvIndex);

	// SRVを生成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	// SRVの設定を行う
	srvDesc.Format = mipImages.GetMetadata().format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	// CubeMapかどうか判定
	if (mipImages.GetMetadata().IsCubemap())
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.MipLevels = UINT_MAX;
		srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
	}
	else
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = UINT(mipImages.GetMetadata().mipLevels);
	}
	
	// srvの生成
	dxBasis_->GetDevice()->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, textureData.srvHandleCPU);

	// テクスチャデータを転送する
	// 転送用に生成した中間リソースをテクスチャデータ構造体に格納
	textureData.intermediateResource = dxBasis_->UploadTextureData(textureData.resource.Get(), mipImages);
}

void TextureManager::LoadRenderTexture(const std::string& filePath)
{
	// 読み込み済みテクスチャを検索
	if (textureDatas.contains(filePath))
	{
		return;
	}

	// テクスチャ枚数上限チェック
	assert(srvManager_->CanAllocate());

	// テクスチャファイルを読み込んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);

	// DDSかどうか判定
	HRESULT hr;
	if (filePathW.ends_with(L".dds"))
	{
		// DDSを読み込む
		hr = DirectX::LoadFromDDSFile(filePathW.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
	}
	else
	{
		hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	}

	assert(SUCCEEDED(hr));

	// ミニマップの作成
	DirectX::ScratchImage mipImages{};

	// 圧縮フォーマットかどうか判定
	if (DirectX::IsCompressed(image.GetMetadata().format))
	{
		// 圧縮フォーマットならそのまま使う
		mipImages = std::move(image);
	}
	else
	{
		hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
		assert(SUCCEEDED(hr));
	}

	// 追加したテクスチャデータの参照を取得する
	RenderTextureData& textureData = renderTextureDatas[filePath];

	// 追加したテクスチャデータに書き込む
	textureData.metaData = mipImages.GetMetadata();
	// クライアント領域のサイズ
	const int32_t kClientWidth = 1280;
	const int32_t kClientHeight = 720;
	const Vector4 kRenderTargetClearValue{ 0.1f,0.25f,0.5f,1.0f };
	textureData.resource = dxBasis_->CreateRenderTextureResource(textureData.metaData, kClientWidth, kClientHeight, kRenderTargetClearValue);

	// テクスチャデータの要素数番号をSRVのインデックスとする
	textureData.srvIndex = srvManager_->Allocate();
	textureData.srvHandleCPU = srvManager_->GetCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = srvManager_->GetGPUDescriptorHandle(textureData.srvIndex);

	// RenderTexture用の設定
	D3D12_SHADER_RESOURCE_VIEW_DESC renderTextureSrvDesc{};
	renderTextureSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	renderTextureSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	renderTextureSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	renderTextureSrvDesc.Texture2D.MipLevels = 1;

	// SRVの生成
	dxBasis_->GetDevice()->CreateShaderResourceView(textureData.resource.Get(), &renderTextureSrvDesc, textureData.srvHandleCPU);

	// Depth用のの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC depthTextureSrvDesc{};
	depthTextureSrvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	depthTextureSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	depthTextureSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	depthTextureSrvDesc.Texture2D.MipLevels = 1;
	// テクスチャデータの要素数番号をSRVのインデックスとする
	textureData.depthSrvIndex = srvManager_->Allocate();
	textureData.depthSrvHandleCPU = srvManager_->GetCPUDescriptorHandle(textureData.depthSrvIndex);
	textureData.depthSrvHandleGPU = srvManager_->GetGPUDescriptorHandle(textureData.depthSrvIndex);
	// SRVの生成
	dxBasis_->GetDevice()->CreateShaderResourceView(dxBasis_->GetDepthResource(), &depthTextureSrvDesc, textureData.depthSrvHandleCPU);

}

void TextureManager::ReleaseIntermediateResources()
{
	for (auto& [key, textureData] : textureDatas)
	{
		if (textureData.intermediateResource) 
		{
			// ComPtrのResetを呼び出してリソースを解放し、nullptrにする
			textureData.intermediateResource.Reset();
		}
	}
}

uint32_t TextureManager::GetTextureIndexByFilePath(const std::string& filePath)
{
	// map からテクスチャを検索
	std::unordered_map<std::string, TextureData>::const_iterator it =
		textureDatas.find(filePath);

	// 存在しなければ停止
	assert(it != textureDatas.end());

	// value の中にある SRV index を返す
	return it->second.srvIndex;

}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSRVHandleGPU(const std::string &filePath)
{
	// 範囲外指定違反チェック
	std::unordered_map<std::string, TextureData>::const_iterator it =
		textureDatas.find(filePath);
    
	// 存在しなければ停止
	assert(it != textureDatas.end());

	// テクスチャデータ（value）を取得
	const TextureData& textureData = it->second;
	
	// そのテクスチャの GPU SRV ハンドルを返す
	return textureData.srvHandleGPU;
}

const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string& filePath)
{
	// キーでテクスチャを検索
	std::unordered_map<std::string, TextureData>::iterator it =
		textureDatas.find(filePath);

	// 見つからなかったら停止
	assert(it != textureDatas.end());

	// テクスチャデータを取得してメタデータを返す
	return it->second.metaData;
}
