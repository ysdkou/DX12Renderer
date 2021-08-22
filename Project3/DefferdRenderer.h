#pragma once
#include "DX12App.h"
#include <DirectXMath.h>
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

	struct MVPCb
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
	};

	struct MaterialCb
	{
		DirectX::XMFLOAT3 albedo;
		float metallic;
		float roughness;
		float ao;
	};

	ComPtr<ID3D12Resource1> m_vertexBuffer;

	ComPtr<ID3D12Resource1> m_indexBuffer;
	UINT m_indexCount;

	D3D12_VERTEX_BUFFER_VIEW  m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW   m_indexBufferView;


	static constexpr int LIGHT_BUFFER_INDEX = 0;
	static constexpr int MVP_BUFFER_START_INDEX = 1;
	static constexpr int MATERIAL_BUFFER_INDEX = 3;
	static constexpr int CB_HEAP_SIZE = MATERIAL_BUFFER_INDEX + 1;
	ComPtr<ID3D12DescriptorHeap> m_cbvHeap;
	UINT m_cbvDiscriptorSize;
	ComPtr<ID3D12Resource1> m_lightBuffer;
	std::vector <ComPtr<ID3D12Resource1>> m_mvpBuffers;
	ComPtr<ID3D12Resource>   m_materialBuffer;

	D3D12_GPU_DESCRIPTOR_HANDLE m_lightBufferView;
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_mvpBufferViews;
	D3D12_GPU_DESCRIPTOR_HANDLE m_materialBufferView;
	
	ComPtr<ID3DBlob> m_vs, m_ps;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12PipelineState> m_basePipeline;

	ComPtr<ID3D12Resource1> CreateBuffer(UINT buffersize, const void* initialValue);
	UINT alignedBufferSizeOf(UINT size);
	void createVertices();
	void createIndices();
	void createCbvHeap();
	void createLight();
	void createMaterial();
	void createMvp();
	void createRootSigunature();
	void compileShader();
	void createPipeLine();
public:
	virtual void prepare()override;
	virtual void cleanup() override;
	virtual void MakeCommand(ComPtr<ID3D12GraphicsCommandList>& command)override;
};

