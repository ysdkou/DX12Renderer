#pragma once
#include "DX12App.h"
#include <DirectXMath.h>
#include <memory>
#include "DescriptorHeap.h"
#include "ConstantBuffer.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
//TODO
//model  viewprojのコンスタントバッファ分離
//最終的に
//ligtview heap
//model heap
//material heap
//view heapにcameraPos追加
//IAにNormal追加
//PBRに対応
//Defferedに対応
class DefferdTest:public DX12App
{
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
private:
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
	};
	struct LightCb
	{
		DirectX::XMFLOAT3 direction;
		DirectX::XMFLOAT3 color;
	};

	struct ViewProjectionCb
	{
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
		DirectX::XMFLOAT3 cameraPos;
	};

	struct MaterialCb
	{
		DirectX::XMFLOAT3 albedo;
		float metallic;
		float roughness;
		float ao;
	};

	struct ModelCb
	{
		DirectX::XMFLOAT4X4 model;
	};

	std::unique_ptr<MyDX12::IndexBuffer> m_indexBuffer;
	std::unique_ptr<MyDX12::VertexBuffer> m_vertexBuffer;

	std::array<ComPtr<ID3D12Resource>, 3> m_gBufferTexture;

	ComPtr<ID3DBlob> m_vs, m_ps;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12PipelineState> m_basePipeline;
	ComPtr<ID3D12PipelineState> m_gBufferPipeline;




	static constexpr int LIGHT_BUFFER_START_INDEX = 0;
	static constexpr int LIGHT_BUFFER_SIZE = 1;
	static constexpr int VIEW_PROJECTION_BUFFER_START_INDEX = LIGHT_BUFFER_START_INDEX + LIGHT_BUFFER_SIZE;
	static constexpr int MATERIAL_BUFFER_START_INDEX = VIEW_PROJECTION_BUFFER_START_INDEX + FRAME_BUFFER_COUNT;
	static constexpr int MATERIAL_BUFFER_SIZE = 1;
	static constexpr int MODEL_BUFFER_START_INDEX = MATERIAL_BUFFER_START_INDEX + MATERIAL_BUFFER_SIZE;
	static constexpr int MODEL_BUFFER_SIZE = 1;
	static constexpr int GBUFFER_SRV_START_INDEX = MODEL_BUFFER_START_INDEX + MODEL_BUFFER_SIZE;
	static constexpr int GBUFFER_SIZE = 3;
	static constexpr int DEPTH_BUFFER_SRV_START_INDEX = GBUFFER_SRV_START_INDEX + GBUFFER_SIZE;
	static constexpr int DEPTH_BUFFER_SRV_SIZE = 1;
	static constexpr int  CB_SRV_HEAP_SIZE = DEPTH_BUFFER_SRV_START_INDEX + DEPTH_BUFFER_SRV_START_INDEX;

	

	std::unique_ptr<MyDX12::DescriptorHeap> m_cbvsrvHeap;
	std::unique_ptr<MyDX12::DescriptorHeap> m_gBufferRtvHeap;

	
	std::unique_ptr<MyDX12::ConstantBuffer> m_lightBuffer;
	std::vector<std::unique_ptr<MyDX12::ConstantBuffer>> m_viewProjectionBuffers;
	std::unique_ptr<MyDX12::ConstantBuffer> m_materialBuffer;
	std::unique_ptr<MyDX12::ConstantBuffer> m_modelBuffer;

	D3D12_GPU_DESCRIPTOR_HANDLE m_lightBufferView;
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_viewProjectionBufferViews;
	D3D12_GPU_DESCRIPTOR_HANDLE m_materialBufferView;
	D3D12_GPU_DESCRIPTOR_HANDLE m_modelBufferView;

	
	

	//1.Albedo,roughness
	//2 Normal,metallic
	//3 ao
	std::array<DXGI_FORMAT, 3> m_gBuffferFormat = { DXGI_FORMAT_R8G8B8A8_UNORM ,DXGI_FORMAT_R8G8B8A8_SNORM ,DXGI_FORMAT_R8G8B8A8_UNORM };
	
	std::array<float, 4> m_clearColor = { 0.0,0.0,0.0,0.0 };


	void createVertices();
	void createIndices();
	void createCbvHeap();
	void createLight();
	void createViewProjection();
	void createMaterial();
	void createModel();
	void createRootSigunature();
	void compileShader();
	void createPipeLine();
	void createGBuferRTV();
	void createDepthSRV();
	void createGBufferPipeLine();
	
public:
	virtual void prepare()override;
	virtual void cleanup() override;
	virtual void MakeCommand(ComPtr<ID3D12GraphicsCommandList>& command)override;
};

