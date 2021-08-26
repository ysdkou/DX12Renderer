#pragma once
#include "d3dx12.h"
#include <d3d12.h>
#include <memory>
#include <bitset>
namespace MyDX12 {
	class DescriptorHeapBuilder
	{
		friend class DescriptorHeap;
	private:
		enum Member
		{
			TYPE,
			NUM,
			FLAGS,
			DEVICE,
		};
		std::bitset<4> m_checkFlag;
		D3D12_DESCRIPTOR_HEAP_TYPE m_type;
		UINT m_numDescriptors;
		D3D12_DESCRIPTOR_HEAP_FLAGS m_flags;
		ID3D12Device* m_device;
	public:
		
		DescriptorHeapBuilder& setHeapType(D3D12_DESCRIPTOR_HEAP_TYPE type);
		DescriptorHeapBuilder& setSize(UINT size);
		DescriptorHeapBuilder& setFlags(D3D12_DESCRIPTOR_HEAP_FLAGS flag);
		DescriptorHeapBuilder& setFlagsShaderVisible();
		DescriptorHeapBuilder& setFlagsNone();
		DescriptorHeapBuilder& setDevice(ID3D12Device* device);
		

	};
	//登録したデバイスより前に開放してください
	//内部でデバイスへのポインタを抱えます。
	class DescriptorHeap {
	public:
		template<class T>
		using ComPtr = Microsoft::WRL::ComPtr<T>;
		using Builder = MyDX12::DescriptorHeapBuilder;
	private:

		DescriptorHeap(const DescriptorHeapBuilder& builder);
		ComPtr<ID3D12DescriptorHeap> m_heap;
		UINT m_descriptorSize;
		ID3D12Device* m_device;
	public:
		static std::unique_ptr<DescriptorHeap>  Create(const DescriptorHeapBuilder& builder);
		const ID3D12DescriptorHeap* getRaw()const;
		ID3D12DescriptorHeap* getRaw();
		void createConstantBufferView(const D3D12_CONSTANT_BUFFER_VIEW_DESC* desc, UINT index);
		CD3DX12_GPU_DESCRIPTOR_HANDLE getGPUHandle(UINT index);
	};
}