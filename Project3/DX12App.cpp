#include "DX12App.h"
#include <Windows.h>
#include <execution>
#include <fstream>

template<class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

ComPtr<IDXGIFactory3> DX12App::createDXGIFactory()
{
	
	ComPtr<IDXGIFactory3> factory;
	UINT dxgiFlag =0;
	dxgiFlag |= DXGI_CREATE_FACTORY_DEBUG;
	CreateDXGIFactory2(dxgiFlag, IID_PPV_ARGS(&factory));
	return factory;
}

void DX12App::enableDebugLayer()
{
	//デバッグレイヤの有効化
	ComPtr<ID3D12Debug> debug;

	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug))))
	{
		debug->EnableDebugLayer();
	}

	//ComPtr<ID3D12Debug3> debug3;
	//debug.As(&debug3);
	//
	//debug3->SetEnableGPUBasedValidation(true);
}

void DX12App::createDevice(ComPtr<IDXGIFactory3>& factory)
{

	//ディスクリートGPUのアダプタを取得
	ComPtr<IDXGIAdapter1> useAdapter;
	{
		UINT adapterIndex = 0;
		ComPtr<IDXGIAdapter1> adapter;
		while (DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndex, &adapter))
		{
			DXGI_ADAPTER_DESC1 desc{};
			adapter->GetDesc1(&desc);
			++adapterIndex;
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				continue;
			}

			auto hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr);
			if (SUCCEEDED(hr))
			{
				break;
			}
		}
		adapter.As(&useAdapter);
	}
	//デバイスの生成
	auto hr = D3D12CreateDevice(useAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device));

	if (FAILED(hr))
	{
		throw std::runtime_error("CreateDevice failed.");
	}
}

void DX12App::createQueue()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc{
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		0,
		D3D12_COMMAND_QUEUE_FLAG_NONE,
		0,
	};

	m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)
	);
}

void DX12App::createSwapChain(HWND hwnd, ComPtr<IDXGIFactory3>& factory)
{
	RECT rect;
	GetClientRect(hwnd, &rect);
	const int width = rect.right - rect.left;
	const int height = rect.bottom - rect.top;
	DXGI_SWAP_CHAIN_DESC1 scDesc{};
	scDesc.BufferCount = FRAME_BUFFER_COUNT;
	scDesc.Width = width;
	scDesc.Height = height;
	scDesc.Format  =  DXGI_FORMAT_R8G8B8A8_UNORM;;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> swapChain;
	auto hr = factory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),
		hwnd,
		&scDesc,
		nullptr,
		nullptr,
		&swapChain
	);
	if (FAILED(hr))
	{
		throw std::runtime_error("CreateSwapChainForHwnd failed.");
	}
	swapChain.As(&m_swapchain);

	m_width = width;
	m_height = height;
}

//ここがおかしい
void DX12App::createRenderTargetView()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		FRAME_BUFFER_COUNT,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		0
	};

	m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_heapRtv));
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	
	m_renderTargets.resize(FRAME_BUFFER_COUNT);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_heapRtv->GetCPUDescriptorHandleForHeapStart());

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; ++i)
	{
		m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets.at(i)));
		m_device->CreateRenderTargetView(m_renderTargets.at(i).Get(), nullptr, rtvHandle);

		rtvHandle.Offset(1, m_rtvDescriptorSize);
	}
}

void DX12App::createDepthStencilView()
{

	//DSV用ディスクリプタヒープ作成
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDisc{
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
		1,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		0
	};

	m_device->CreateDescriptorHeap(&dsvHeapDisc, IID_PPV_ARGS(&m_heapDsv));

	//デプスバッファのメモリーリソース作成


	auto resDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_D32_FLOAT,
		m_width, m_height,
		1, 0,
		1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
	);

	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.Format = resDesc.Format;
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.DepthStencil.Stencil = 0;

	//左辺値からアドレスは取れない
	CD3DX12_HEAP_PROPERTIES heapProperty(D3D12_HEAP_TYPE_DEFAULT);

	m_device->CreateCommittedResource(
		&heapProperty,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(&m_depthStencil)
	);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc
	{
		DXGI_FORMAT_D32_FLOAT,
		D3D12_DSV_DIMENSION_TEXTURE2D,
		D3D12_DSV_FLAG_NONE,
		{  //D3D12_TEX2D_DSV
			0// MipSlice
		}
	};

	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandlle(m_heapDsv->GetCPUDescriptorHandleForHeapStart());

	m_device->CreateDepthStencilView(m_depthStencil.Get(), &dsvDesc, dsvHandlle);

}

void DX12App::createCommandAllocator()
{
	m_commandAllocators.resize(FRAME_BUFFER_COUNT);

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; ++i)
	{
		m_device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&m_commandAllocators.at(i))
		);
	}
}

void DX12App::createFrameFences()
{
	m_frameFences.resize(FRAME_BUFFER_COUNT);

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_frameFences.at(i)));
	}
}

void DX12App::createCommandList()
{
	m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_commandAllocators.at(0).Get(), nullptr, IID_PPV_ARGS(&m_commandList));
	m_commandList->Close();
}

void DX12App::setupViewportScissor()
{
	m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, float(m_width), float(m_height));
	m_scissorRect = CD3DX12_RECT(0, 0, LONG(m_width), LONG(m_height));
}

void DX12App::waitPreviousFrame()
{
	auto& fence = m_frameFences[m_frameIndex];
	const auto currentValue = ++m_frameFenceValue[m_frameIndex];
	m_commandQueue->Signal(fence.Get(), currentValue);

	auto nextIndex = (m_frameIndex + 1) % FRAME_BUFFER_COUNT;
	
	const auto finishExpected =m_frameFenceValue[nextIndex];
	const auto nextFenceValue = m_frameFences.at(nextIndex)->GetCompletedValue();
	//wait before frame signal
	if (nextFenceValue < finishExpected)
	{
		m_frameFences.at(nextIndex)->SetEventOnCompletion(finishExpected, m_fenceWaitEvent);
		WaitForSingleObject(m_fenceWaitEvent, GPU_WAIT_TIME_OUT);
	}
}

HRESULT DX12App::CompileShaderFromFile(const std::wstring& filename, const std::wstring& profile, ComPtr<ID3DBlob>& shaderBlob, ComPtr<ID3DBlob>& errorBlob)
{
	using namespace std::experimental::filesystem;

	path filePath(filename);
	std::ifstream infile(filePath);
	std::vector<char> srcData;
	if (!infile)
		throw std::runtime_error("shader not found");
	srcData.resize(uint32_t(infile.seekg(0, infile.end).tellg()));
	infile.seekg(0, infile.beg).read(srcData.data(), srcData.size());

	// DXC によるコンパイル処理
	ComPtr<IDxcLibrary> library;
	ComPtr<IDxcCompiler> compiler;
	ComPtr<IDxcBlobEncoding> source;
	ComPtr<IDxcOperationResult> dxcResult;

	DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library));
	library->CreateBlobWithEncodingFromPinned(srcData.data(), UINT(srcData.size()), CP_ACP, &source);
	DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));

	LPCWSTR compilerFlags[] = {
  #if _DEBUG
	  L"/Zi", L"/O0",
  #else
	  L"/O2" // リリースビルドでは最適化
  #endif
	};
	compiler->Compile(source.Get(), filePath.wstring().c_str(),
		L"main", profile.data(),
		compilerFlags, _countof(compilerFlags),
		nullptr, 0, // Defines
		nullptr,
		&dxcResult);

	HRESULT hr;
	dxcResult->GetStatus(&hr);
	if (SUCCEEDED(hr))
	{
		dxcResult->GetResult(
			reinterpret_cast<IDxcBlob**>(shaderBlob.GetAddressOf())
		);
	}
	else
	{
		dxcResult->GetErrorBuffer(
			reinterpret_cast<IDxcBlobEncoding**>(errorBlob.GetAddressOf())
		);
	}
	return hr;
}

DX12App::DX12App():
	m_height(0),
	m_width(0),
	m_rtvDescriptorSize(0),
	m_frameIndex(0),
	m_fenceWaitEvent(NULL),
	m_frameFenceValue({0,0})
{

}

DX12App::~DX12App()
{
}

void DX12App::intialize(HWND hWnd)
{
	enableDebugLayer();
	auto factory = createDXGIFactory();
	createDevice(factory);
	createQueue();
	createSwapChain(hWnd, factory);
	createRenderTargetView();
	createDepthStencilView();
	createCommandAllocator();
	createFrameFences();
	createCommandList();
	setupViewportScissor();
	prepare();
}

void DX12App::terminate()
{
}

void DX12App::render()
{
	//この時点で前フレ描画の同期とると描画完了が確定しているのでコンスタントバッファをシングルバッファで運用できる。
	m_frameIndex = m_swapchain->GetCurrentBackBufferIndex();
	//OutputDebugStringA(std::to_string(m_frameIndex).c_str());
	//OutputDebugStringA("\n");

	m_commandAllocators[m_frameIndex]->Reset();
	m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(m_heapRtv->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);

	CD3DX12_CPU_DESCRIPTOR_HANDLE dsv(m_heapDsv->GetCPUDescriptorHandleForHeapStart());

	auto barrierToRT = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets.at(m_frameIndex).Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_commandList->ResourceBarrier(1, &barrierToRT);

	const float clearColor[] = { 0.25,0.25,0.25,1 };
	m_commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);

	m_commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	
	m_commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

	MakeCommand(m_commandList);

	auto barrierToPresent = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets.at(m_frameIndex).Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	
	m_commandList->ResourceBarrier(1, &barrierToPresent);
	m_commandList->Close();

	ID3D12CommandList* lists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(1, lists);

	m_swapchain->Present(1, 0);


	//この時点で同期をとるとCB更新時に前触れの描画が完了していなかったりするので問題が出る。
	waitPreviousFrame();
}

void DX12App::MakeCommand(ComPtr<ID3D12GraphicsCommandList>& command)
{
}
