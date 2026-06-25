#pragma once
#include <string>
#include <unordered_map>
#include <d3d12.h>
#include <externals/DirectXTex/DirectXTex.h>
#include"externals/DirectXTex/d3dx12.h"
#include <wrl.h>
#include "DirectXBasis.h"
#include "SrvManager.h"

class TextureManager
{
private:
	static TextureManager* instance;

	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = delete;
	TextureManager& operator=(TextureManager&) = delete;

	// テクスチャ1枚分のデータ
	struct TextureData
	{
		DirectX::TexMetadata metaData;
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		uint32_t srvIndex;
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;
		Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource;
	};

	struct RenderTextureData
	{
		DirectX::TexMetadata metaData;
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		uint32_t srvIndex;
		Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource;
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;
		uint32_t depthSrvIndex;
		D3D12_CPU_DESCRIPTOR_HANDLE depthSrvHandleCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE depthSrvHandleGPU;
	};

	// テクスチャデータ
	std::unordered_map<std::string, TextureData> textureDatas;
	std::unordered_map<std::string, RenderTextureData> renderTextureDatas;

	// ポインタ
	DirectXBasis* dxBasis_ = nullptr;
	SrvManager* srvManager_ = nullptr;

	// SRVインデックスの開始番号
	static uint32_t kSRVIndexTop;

public:
	// シングルトンインスタンスの取得
	static TextureManager* GetInstance();
	// 終了
	void Finalize();
	// 初期化
	void Initialize(DirectXBasis* dxBasis,SrvManager* srvManager);

	// LoadTexture関数
	void LoadTexture(const std::string& filePath);

	// LoadRenderTexture関数
	void LoadRenderTexture(const std::string& filePath);

	// 中間リソースを開放
	void ReleaseIntermediateResources();

	// SRVインデックスの開始番号
	uint32_t GetTextureIndexByFilePath(const std::string& filePath);

	// テクスチャ番号からGPUハンドルを取得する
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVHandleGPU(const std::string& filePath);
	D3D12_GPU_DESCRIPTOR_HANDLE GetRenderSRVHandleGPU(const std::string& filePath) 
	{
		return renderTextureDatas[filePath].srvHandleGPU;
	}
	D3D12_GPU_DESCRIPTOR_HANDLE GetDepthSRVHandle(const std::string& filePath)
	{
		return renderTextureDatas[filePath].depthSrvHandleGPU;
	}

	// メタデータを取得
	const DirectX::TexMetadata& GetMetaData(const std::string& filePath);

	// テクスチャデータを取得
	ID3D12Resource* GetRenderTextureData(const std::string& filePath) { return renderTextureDatas[filePath].resource.Get(); }

};

