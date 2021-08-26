#pragma once
#include "d3dx12.h"
#include <d3d12.h>
#include <memory>
#include <bitset>

namespace MyDX12
{
	class ConstantbufferBuilder
	{
	private:
		friend class ConstantBuffer;
		UINT m_size;
		const void* m_initalValue;
		ID3D12Device* m_device;
		std::bitset<2> m_checkFlag;
		enum Member
		{
			SIZE,
			DEVICE,
		};
	public:
		ConstantbufferBuilder() :m_initalValue(nullptr), m_size(0), m_device(nullptr) {}

		template<class T>
		ConstantbufferBuilder& setSizeAndValueFromInstance(T&& t)
		{
			m_size = sizeof(T);
			m_initalValue = &t;

			m_checkFlag.set(SIZE);
			return *this;
		}
		template<class T>
		ConstantbufferBuilder& setSizeFromType()
		{
			m_size = sizeof(T);
			m_checkFlag.set(SIZE);
			return *this;
		}

		ConstantbufferBuilder& setSize(UINT size);
		ConstantbufferBuilder& setInitalValue(const void* value);
		ConstantbufferBuilder& seDevice(ID3D12Device* device);


	};
	//サイズは自動的にアライメントします。（メモリ食うので対策考えたい）
	class ConstantBuffer
	{
	private:
		UINT m_size;
		ID3D12Device* m_device;
		Microsoft::WRL::ComPtr<ID3D12Resource1> m_buffer;
		ConstantBuffer(const ConstantbufferBuilder& builder);
	public:
		using Builder = ConstantbufferBuilder;
		static std::unique_ptr<ConstantBuffer> Create(const  ConstantbufferBuilder& builder);

		D3D12_CONSTANT_BUFFER_VIEW_DESC createViewDesc();
	};

}

