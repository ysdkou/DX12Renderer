#include "VertexBuffer.h"
#include <assert.h>
namespace MyDX12
{
	VertexBufferBuilder::VertexBufferBuilder() :m_size(0), m_stride(0), m_device(nullptr), m_initialValue(nullptr)
	{
	}
	VertexBufferBuilder& VertexBufferBuilder::setSize(UINT size)
	{
		m_checkFlag.set(SIZE);
		m_size = size;
		return *this;
	}
	VertexBufferBuilder& VertexBufferBuilder::setStride(UINT stride)
	{
		m_checkFlag.set(STRIDE);
		m_stride = stride;
		return *this;
	}
	VertexBufferBuilder& VertexBufferBuilder::setInitialValue(const void* value)
	{
		m_initialValue = value;
		return *this;
	}
	VertexBufferBuilder& VertexBufferBuilder::setDevice(ID3D12Device* device)
	{
		m_checkFlag.set(DEVICE);
		m_device = device;
		return *this;
	}
	VertexBuffer::VertexBuffer(const VertexBufferBuilder& builder):
		m_size(builder.m_size),
		m_stride(builder.m_stride),
		m_device(builder.m_device)
	{
		HRESULT hr;
		const auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		const auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(builder.m_size);
		hr = m_device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_buffer)
		);

		if (SUCCEEDED(hr) && builder.m_initialValue != nullptr)
		{
			void* mapped;
			CD3DX12_RANGE range(0, 0);
			hr = m_buffer->Map(0, &range, &mapped);
			if (SUCCEEDED(hr))
			{
				memcpy(mapped, builder.m_initialValue,builder.m_size);
				m_buffer->Unmap(0, nullptr);
			}
		}

	}
	D3D12_VERTEX_BUFFER_VIEW VertexBuffer::getBufferView()
	{
		D3D12_VERTEX_BUFFER_VIEW result;
		result.BufferLocation  = m_buffer->GetGPUVirtualAddress();
		result.SizeInBytes = m_size;
		result.StrideInBytes = m_stride;
		return result;
	}
	std::unique_ptr<VertexBuffer> VertexBuffer::Create(const VertexBufferBuilder& builder)
	{
		assert(builder.m_checkFlag.all() && "VertexBuffer‚Ìˆê•”ƒƒ“ƒo‚ªİ’è‚³‚ê‚Ä‚¢‚Ü‚¹‚ñ");
		return std::unique_ptr<VertexBuffer>(new VertexBuffer(builder));
	}


}