#pragma once
#include "d3dx12.h"
#include <d3d12.h>
#include <memory>
#include <bitset>

namespace MyDX12
{
	class VertexBufferBuilder
	{
	private:
		friend class VertexBuffer;
		UINT m_size;
		UINT m_stride;
		ID3D12Device* m_device;
		const void* m_initialValue;
		enum Member
		{
			SIZE,
			STRIDE,
			DEVICE,
		};

		std::bitset<3> m_checkFlag;

	public:
		VertexBufferBuilder();
		VertexBufferBuilder& setSize(UINT size);
		VertexBufferBuilder& setStride(UINT stride);
		VertexBufferBuilder& setInitialValue(const void* value);
		VertexBufferBuilder& setDevice(ID3D12Device* device);
	};

	class VertexBuffer
	{

	private:
		UINT m_size;
		UINT m_stride;
		ID3D12Device* m_device;
		Microsoft::WRL::ComPtr<ID3D12Resource1> m_buffer;
		VertexBuffer(const VertexBufferBuilder& builder);
	public:
		using Builder = VertexBufferBuilder;
		D3D12_VERTEX_BUFFER_VIEW getBufferView();
		static std::unique_ptr<VertexBuffer> Create(const VertexBufferBuilder& builder);

	};
}