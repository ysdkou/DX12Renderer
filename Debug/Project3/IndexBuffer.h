#pragma once
#include "d3dx12.h"
#include <d3d12.h>
#include <memory>
#include <bitset>
namespace MyDX12
{
	class IndexBufferBuilder
	{
	private:
		friend class IndexBuffer;
		UINT m_size;
		UINT m_sizePerIndex;
		const void* m_value;
		ID3D12Device* m_device;
		enum Member
		{
			SIZE,
			SIZE_PER_INDEX,
			DEVICE 
		};
		std::bitset<3> m_checkFlag;
	public:
		IndexBufferBuilder& setSizeInBytes(UINT size);
		IndexBufferBuilder& setSizePerIndex(UINT size);
		IndexBufferBuilder& setDevice(ID3D12Device* device);
		IndexBufferBuilder& setInitialValue(const void* value);

	};
	class IndexBuffer
	{
	private:
		UINT m_size;
		UINT m_indexCount;
		ID3D12Device* m_device;
		Microsoft::WRL::ComPtr<ID3D12Resource1> m_buffer;
		IndexBuffer(const IndexBufferBuilder& builder);
	public:
		using Builder = IndexBufferBuilder;

		UINT getCount()const;
		D3D12_INDEX_BUFFER_VIEW getBufferView();

		static std::unique_ptr<IndexBuffer> Create(const IndexBufferBuilder& builder);
	};
}


