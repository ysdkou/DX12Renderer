#pragma once
#include "DX12App.h"
#include <DirectXMath.h>
#include <memory>
#include "DescriptorHeap.h"
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

	ComPtr<ID3D12Resource1> m_vertexBuffer;

	ComPtr<ID3D12Resource1> m_indexBuffer;
	UINT m_indexCount;

	D3D12_VERTEX_BUFFER_VIEW  m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW   m_indexBufferView;

	ComPtr<ID3DBlob> m_vs, m_ps;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12PipelineState> m_basePipeline;

	D3D12_GPU_DESCRIPTOR_HANDLE m_lightBufferView;
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_viewProjectionBufferViews;
	D3D12_GPU_DESCRIPTOR_HANDLE m_materialBufferView;
	D3D12_GPU_DESCRIPTOR_HANDLE m_modelBufferView;

	static constexpr int LIGHT_BUFFER_START_INDEX = 0;
	static constexpr int LIGHT_BUFFER_SIZE = 1;
	static constexpr int VIEW_PROJECTION_BUFFER_START_INDEX = 1;
	static constexpr int MATERIAL_BUFFER_START_INDEX = VIEW_PROJECTION_BUFFER_START_INDEX + FRAME_BUFFER_COUNT;
	static constexpr int MATERIAL_BUFFER_SIZE = 1;
	static constexpr int MODEL_BUFFER_START_INDEX = MATERIAL_BUFFER_START_INDEX + MATERIAL_BUFFER_SIZE;
	static constexpr int MODEL_BUFFER_SIZE = 1;
	static constexpr int CB_HEAP_SIZE = MODEL_BUFFER_START_INDEX + MODEL_BUFFER_SIZE;

	//ComPtr<ID3D12DescriptorHeap> m_cbvHeap;
	//UINT m_cbvDiscriptorSize;
	std::unique_ptr<MyDX12::DescriptorHeap> m_cbvsrvHeap;
	ComPtr<ID3D12Resource1> m_lightBuffer;
	std::vector <ComPtr<ID3D12Resource1>> m_viewProjectionBuffers;
	ComPtr<ID3D12Resource>   m_materialBuffer;
	ComPtr<ID3D12Resource>   m_modelBuffer;

	

	ComPtr<ID3D12Resource1> CreateBuffer(UINT buffersize, const void* initialValue);
	UINT alignedBufferSizeOf(UINT size);
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
public:
	virtual void prepare()override;
	virtual void cleanup() override;
	virtual void MakeCommand(ComPtr<ID3D12GraphicsCommandList>& command)override;
};

