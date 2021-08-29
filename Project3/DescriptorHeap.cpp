#include "DescriptorHeap.h"
#include <assert.h>
namespace MyDX12
{
	DescriptorHeapBuilder& DescriptorHeapBuilder::setHeapType(D3D12_DESCRIPTOR_HEAP_TYPE type)
	{
		m_checkFlag.set(TYPE);
		m_type = type;
		return *this;
	
	}
	DescriptorHeapBuilder& DescriptorHeapBuilder::setSize(UINT size)
	{
		m_checkFlag.set(NUM);
		m_numDescriptors = size;
		return *this;
	}
	DescriptorHeapBuilder& DescriptorHeapBuilder::setFlags(D3D12_DESCRIPTOR_HEAP_FLAGS flag)
	{
		m_checkFlag.set(FLAGS);
		m_flags = flag;
		return *this;
	}
	DescriptorHeapBuilder& DescriptorHeapBuilder::setFlagsShaderVisible()
	{
		m_checkFlag.set(FLAGS);
		m_flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		return *this;
	}
	DescriptorHeapBuilder& DescriptorHeapBuilder::setFlagsNone()
	{
		m_checkFlag.set(FLAGS);
		m_flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		return *this;
	}
	DescriptorHeapBuilder& DescriptorHeapBuilder::setDevice(ID3D12Device* device)
	{
		m_checkFlag.set(DEVICE);
		m_device = device;
		return *this;
	}
	DescriptorHeap::DescriptorHeap(const DescriptorHeapBuilder& builder)
	{
		m_device = builder.m_device;

		D3D12_DESCRIPTOR_HEAP_DESC desc
		{
			builder.m_type,
			builder.m_numDescriptors,
			builder.m_flags,
			0
		};

		m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_heap));
		m_descriptorSize = m_device->GetDescriptorHandleIncrementSize(builder.m_type);
		

	}
	std::unique_ptr<DescriptorHeap> DescriptorHeap::Create(const DescriptorHeapBuilder& builder)
	{
		assert(builder.m_checkFlag.all() && "DescriptorHeapBulder‚Ìƒƒ“ƒo‚ªˆê•”İ’è‚³‚ê‚Ä‚¢‚Ü‚¹‚ñ");

		return std::unique_ptr<DescriptorHeap>( new DescriptorHeap(builder));
	}
	const ID3D12DescriptorHeap* DescriptorHeap::getRaw() const
	{
		return m_heap.Get();
	}
	ID3D12DescriptorHeap* DescriptorHeap::getRaw()
	{
		return m_heap.Get();
	}
	void DescriptorHeap::createConstantBufferView(const D3D12_CONSTANT_BUFFER_VIEW_DESC* desc, UINT index)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_heap->GetCPUDescriptorHandleForHeapStart(), index, m_descriptorSize);
		m_device->CreateConstantBufferView(desc, cpuHandle);
	}
	void DescriptorHeap::createRenderTargetView(ID3D12Resource* renderTargetResource, const D3D12_RENDER_TARGET_VIEW_DESC* desc, UINT index)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_heap->GetCPUDescriptorHandleForHeapStart(), index, m_descriptorSize);
		m_device->CreateRenderTargetView(renderTargetResource, desc, cpuHandle);
	}
	void DescriptorHeap::createShaderResouceView(ID3D12Resource* shaderResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* desc, UINT index)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_heap->GetCPUDescriptorHandleForHeapStart(), index, m_descriptorSize);
		m_device->CreateShaderResourceView(shaderResource, desc, cpuHandle);
	}
	CD3DX12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::getGPUHandle(UINT index)
	{
		return CD3DX12_GPU_DESCRIPTOR_HANDLE(m_heap->GetGPUDescriptorHandleForHeapStart(), index, m_descriptorSize);
	}
	CD3DX12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::getCPUHandle(UINT index)
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_heap->GetCPUDescriptorHandleForHeapStart(), index, m_descriptorSize);
	}
}

