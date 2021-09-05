#include "ConstantBuffer.h"
#include <assert.h>
namespace MyDX12 {
	namespace
	{
		//ConstantBufferは255アライメント
		UINT Align(UINT size)
		{
			return  size + 255 & ~255;
		}
	}
	ConstantbufferBuilder& ConstantbufferBuilder::setSize(UINT size)
	{
		m_size = size;
		m_checkFlag.set(SIZE);
		return *this;
	}
	ConstantbufferBuilder& ConstantbufferBuilder::setInitalValue(const void* value)
	{
		m_initalValue = value;
		return *this;
	}
	ConstantbufferBuilder& ConstantbufferBuilder::seDevice(ID3D12Device* device)
	{
		m_device = device;
		m_checkFlag.set(DEVICE);
		return *this;
		// TODO: return ステートメントをここに挿入します
	}
	ConstantBuffer::ConstantBuffer(const ConstantbufferBuilder& builder):
		m_size(Align(builder.m_size)),
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

		if (SUCCEEDED(hr) && builder.m_initalValue != nullptr)
		{
			void* mapped;
			CD3DX12_RANGE range(0, 0);
			hr = m_buffer->Map(0, &range, &mapped);
			if (SUCCEEDED(hr))
			{
				memcpy(mapped, builder.m_initalValue, builder.m_size);
				m_buffer->Unmap(0, nullptr);
			}
		}
	}
	std::unique_ptr<ConstantBuffer> ConstantBuffer::Create(const ConstantbufferBuilder& builder)
	{
		assert(builder.m_checkFlag.all() && "ConstantBufferBulderのメンバが一部設定されていません");
		return std::unique_ptr<ConstantBuffer>(new ConstantBuffer(builder));
	}
	D3D12_CONSTANT_BUFFER_VIEW_DESC ConstantBuffer::createViewDesc()
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC result{};
		result.BufferLocation  = m_buffer->GetGPUVirtualAddress();
		result.SizeInBytes = m_size;
		return result;
	}
}