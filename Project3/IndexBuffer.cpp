#include "IndexBuffer.h"
#include <assert.h>
namespace MyDX12
{
	IndexBufferBuilder& IndexBufferBuilder::setSizeInBytes(UINT size)
	{
		m_checkFlag.set(SIZE);
		m_size = size;
		return *this;
	}
	IndexBufferBuilder& IndexBufferBuilder::setSizePerIndex(UINT size)
	{
		m_checkFlag.set(SIZE_PER_INDEX);
		m_sizePerIndex = size;
		return *this;
	}
	IndexBufferBuilder& IndexBufferBuilder::setDevice(ID3D12Device* device)
	{
		m_checkFlag.set(DEVICE);
		m_device = device;
		return *this;
	}
	IndexBufferBuilder& IndexBufferBuilder::setInitialValue(const void* value)
	{
		m_value = value;
		return *this;
	}
	IndexBuffer::IndexBuffer(const IndexBufferBuilder& builder):
		m_size(builder.m_size),
		m_indexCount(builder.m_size/builder.m_sizePerIndex),
		m_device(builder.m_device)
	{
		HRESULT hr;

		const auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		const auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(m_size);
		hr = m_device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_buffer)
		);

		if (SUCCEEDED(hr) && builder.m_value != nullptr)
		{
			void* mapped;
			CD3DX12_RANGE range(0, 0);
			hr = m_buffer->Map(0, &range, &mapped);
			if (SUCCEEDED(hr))
			{
				memcpy(mapped, builder.m_value, builder.m_size);
				m_buffer->Unmap(0, nullptr);
			}
		}
	}
	UINT IndexBuffer::getCount() const
	{
		return m_indexCount;
	}
	D3D12_INDEX_BUFFER_VIEW IndexBuffer::getBufferView()
	{
		D3D12_INDEX_BUFFER_VIEW result{};
		result.BufferLocation = m_buffer->GetGPUVirtualAddress();
		result.SizeInBytes = m_size;
		result.Format = DXGI_FORMAT_R32_UINT;
		return result;
	}
	std::unique_ptr<IndexBuffer> IndexBuffer::Create(const IndexBufferBuilder& builder)
	{
		assert(builder.m_checkFlag.all() && "IndexBufferBuilder‚Ìƒƒ“ƒo‚ªˆê•”İ’è‚³‚ê‚Ä‚¢‚Ü‚¹‚ñ");
		return std::unique_ptr<IndexBuffer>(new IndexBuffer(builder));
	}
}