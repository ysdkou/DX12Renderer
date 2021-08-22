#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>

#include "d3dx12.h"
#include <wrl.h>
#include <vector>
#include <array>

#if _MSC_VER > 1922
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#endif
#include <experimental/filesystem>


// DirectX Shader Compiler óp(ëÊ7èÕÇ≈ê‡ñæ)
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")


#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
class DX12App
{
public:
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
protected:

	UINT m_width;
	UINT m_height;
	
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<IDXGISwapChain4> m_swapchain;

	ComPtr<ID3D12DescriptorHeap> m_heapRtv;
	UINT m_rtvDescriptorSize;
	std::vector<ComPtr<ID3D12Resource>> m_renderTargets;

	ComPtr<ID3D12DescriptorHeap> m_heapDsv;
	ComPtr<ID3D12Resource> m_depthStencil;
	
	std::vector<ComPtr<ID3D12CommandAllocator>> m_commandAllocators;

	std::vector<ComPtr<ID3D12Fence>> m_frameFences;

	UINT m_frameIndex;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	
	std::array<UINT, 2> m_frameFenceValue;
	HANDLE m_fenceWaitEvent;

	CD3DX12_VIEWPORT  m_viewport;
	CD3DX12_RECT m_scissorRect;

	ComPtr<IDXGIFactory3> createDXGIFactory();



	void enableDebugLayer();
	void createDevice(ComPtr<IDXGIFactory3>& factory);
	void createQueue();
	void createSwapChain(HWND hwnd, ComPtr<IDXGIFactory3>& factory);
	void createRenderTargetView();
	void createDepthStencilView();
	void createCommandAllocator();
	void createFrameFences();
	void createCommandList();
	void setupViewportScissor();
	void waitPreviousFrame();

	HRESULT CompileShaderFromFile(const std::wstring& filename, const std::wstring& profile, ComPtr<ID3DBlob>& shaderBlob, ComPtr<ID3DBlob>& errorBlob);
	static constexpr UINT GPU_WAIT_TIME_OUT = (10 * 1000);
	static constexpr UINT FRAME_BUFFER_COUNT = 2;
public:
	
	DX12App();
	virtual ~DX12App();

	void intialize(HWND hWnd);
	void terminate();

	virtual void render();

	virtual void prepare() {};
	virtual void cleanup() {};
	virtual void MakeCommand(ComPtr<ID3D12GraphicsCommandList>& command);
};

