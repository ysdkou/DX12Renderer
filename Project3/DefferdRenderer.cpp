#include "DefferdRenderer.h"

void DefferdTest::createCbvHeap()
{
	//CBV
	//0,Light
	//1,2 MVP (frame count分)
	//3 Material

	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc{
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		CB_HEAP_SIZE,
		D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
		0
	};

	m_device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_cbvHeap));
	m_cbvDiscriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

}
void DefferdTest::createLight()
{
	LightCb light = { {-1.0f,0.0f,0.0f},{1.0f,1.0f,1.0f} };
	UINT bufferSize = alignedBufferSizeOf(sizeof(LightCb));
	m_lightBuffer = CreateBuffer(bufferSize, &light);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbDesc{};
	cbDesc.BufferLocation = m_lightBuffer->GetGPUVirtualAddress();
	cbDesc.SizeInBytes = bufferSize;

	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_cbvHeap->GetCPUDescriptorHandleForHeapStart(), LIGHT_BUFFER_INDEX, m_cbvDiscriptorSize);

	m_device->CreateConstantBufferView(&cbDesc, cpuHandle);

	m_lightBufferView = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_cbvHeap->GetGPUDescriptorHandleForHeapStart(), LIGHT_BUFFER_INDEX, m_cbvDiscriptorSize);

}
void DefferdTest::createMaterial() {

	MaterialCb material = { {0.9f,0.0f,0.0f},1.0f,1.0f,1.0f };
	UINT bufferSize = alignedBufferSizeOf(sizeof(MaterialCb));
	m_materialBuffer = CreateBuffer(bufferSize, &material);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbDesc{};
	cbDesc.BufferLocation = m_materialBuffer->GetGPUVirtualAddress();
	cbDesc.SizeInBytes = bufferSize;

	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_cbvHeap->GetCPUDescriptorHandleForHeapStart(), MATERIAL_BUFFER_INDEX, m_cbvDiscriptorSize);

	m_device->CreateConstantBufferView(&cbDesc, cpuHandle);

	m_materialBufferView = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_cbvHeap->GetGPUDescriptorHandleForHeapStart(), MATERIAL_BUFFER_INDEX, m_cbvDiscriptorSize);
}
void DefferdTest::createMvp()
{
	m_mvpBuffers.resize(FRAME_BUFFER_COUNT);
	m_mvpBufferViews.resize(FRAME_BUFFER_COUNT);
	for (int i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		using namespace DirectX;

		//各行列のセット.
		MVPCb mvp;
		XMStoreFloat4x4(&mvp.model, XMMatrixRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMConvertToRadians(45.0f)));
		auto mtxView = XMMatrixLookAtLH(
			XMVectorSet(0.0f, 3.0f, -5.0f, 0.0f),
			XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
			XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
		);
		auto mtxProj = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), 480.0f / 360.0f, 0.1f, 100.0f);
		XMStoreFloat4x4(&mvp.view, XMMatrixTranspose(mtxView));
		XMStoreFloat4x4(&mvp.projection, XMMatrixTranspose(mtxProj));
		;

		UINT bufferSize = alignedBufferSizeOf(sizeof(MVPCb));
		m_mvpBuffers[i] = CreateBuffer(bufferSize, &mvp);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbDesc{};
		cbDesc.BufferLocation = m_mvpBuffers[i]->GetGPUVirtualAddress();
		cbDesc.SizeInBytes = bufferSize;

		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_cbvHeap->GetCPUDescriptorHandleForHeapStart(), MVP_BUFFER_START_INDEX + i, m_cbvDiscriptorSize);

		m_device->CreateConstantBufferView(&cbDesc, cpuHandle);

		m_mvpBufferViews[i] = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_cbvHeap->GetGPUDescriptorHandleForHeapStart(), MVP_BUFFER_START_INDEX +i, m_cbvDiscriptorSize);
	}

}

DefferdTest::ComPtr<ID3D12Resource1> DefferdTest::CreateBuffer(UINT buffersize, const void* initialValue)
{
	HRESULT hr;
	ComPtr<ID3D12Resource1> buffer;
	const auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	const auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(buffersize);
	hr = m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&buffer)
	);

	if (SUCCEEDED(hr) && initialValue != nullptr)
	{
		void* mapped;
		CD3DX12_RANGE range(0, 0);
		hr = buffer->Map(0, &range, &mapped);
		if (SUCCEEDED(hr))
		{
			memcpy(mapped, initialValue, buffersize);
			buffer->Unmap(0, nullptr);
		}
	}

	return buffer;
}

UINT DefferdTest::alignedBufferSizeOf(UINT size)
{
	return size + 255 &~255;
}

void DefferdTest::createVertices()
{
	const float k = 1.0f;

	Vertex triangleVertices[] = {
		// 正面
		{{-k ,k, - k,}},
		{{-k, k,-k}} ,
		{ { k, k,-k}} ,
		{{ k,-k,-k}} ,
		  // 右
		{{ k,-k,-k}} ,
		{{ k, k,-k}},
		{{ k, k, k}},
		{{ k,-k, k}},
		  // 左
		{{-k,-k, k}},
		{{-k, k, k}},
		{{-k, k,-k}},
		{{-k,-k,-k}},
		  // 裏
		{{ k,-k, k}},
		{{ k, k, k}},
		{{-k, k, k}},
		{{-k,-k, k}},
		  // 上
		{{-k, k,-k}},
		{{-k, k, k}},
		{{ k, k, k}},
		{{ k, k,-k}},
		  // 底,
		{{-k,-k, k}},
		{{-k,-k,-k}},
		{{ k,-k,-k}},
		{{ k,-k, k}}
	};

	m_vertexBuffer = CreateBuffer(sizeof(triangleVertices), triangleVertices);

	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = sizeof(triangleVertices);
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);
}



void DefferdTest::createIndices()
{
	uint32_t indices[] = {
		0, 1, 2, 2, 3,0,
		4, 5, 6, 6, 7,4,
		8, 9, 10, 10, 11, 8,
		12,13,14, 14,15,12,
		16,17,18, 18,19,16,
		20,21,22, 22,23,20,
	};

	m_indexBuffer = CreateBuffer(sizeof(indices), indices);

	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.SizeInBytes = sizeof(indices);
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	
	m_indexCount = sizeof(indices);
}

void DefferdTest::createRootSigunature()
{
	CD3DX12_DESCRIPTOR_RANGE light, mvp, material;
	light.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	mvp.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
    material.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);

	CD3DX12_ROOT_PARAMETER rootParams[3];

	rootParams[0].InitAsDescriptorTable(1, &light);
	rootParams[1].InitAsDescriptorTable(1, &mvp);
	rootParams[2].InitAsDescriptorTable(1, &material);

	CD3DX12_ROOT_SIGNATURE_DESC desc;

	desc.Init(_countof(rootParams), rootParams, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> errBlob;
	auto hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signature, &errBlob);

	if (FAILED(hr))
	{
		throw std::runtime_error("D3D12RootSignatureFailed");
	}

	hr = m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));

	if (FAILED(hr))
	{
		throw std::runtime_error("Create Root SIgnature Failed");
	}

}

void DefferdTest::compileShader()
{
	// シェーダーをコンパイル.
	HRESULT hr;
	ComPtr<ID3DBlob> errBlob;
	hr = CompileShaderFromFile(L"DefferedTestVertex.hlsl", L"vs_6_0", m_vs, errBlob);
	if (FAILED(hr))
	{
		OutputDebugStringA((const char*)errBlob->GetBufferPointer());
		throw std::runtime_error("Shader Comple Failed");
	}
	hr = CompileShaderFromFile(L"DefferedTestPixel.hlsl", L"ps_6_0", m_ps, errBlob);
	if (FAILED(hr))
	{
		OutputDebugStringA((const char*)errBlob->GetBufferPointer());
		throw std::runtime_error("Shader Comple Failed");
	}
}

void DefferdTest::createPipeLine()
{
	// インプットレイアウト
	D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
	  { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA},
	};


	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};

	psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vs.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_ps.Get());

	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

	auto rasterizerDesc =  CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.RasterizerState = rasterizerDesc;
		

	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	// デプスバッファのフォーマットを設定
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	auto depthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	depthStencilDesc.DepthEnable = FALSE;
	psoDesc.DepthStencilState = depthStencilDesc;

	psoDesc.InputLayout = { inputElementDesc, _countof(inputElementDesc) };

	// ルートシグネチャのセット
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// マルチサンプル設定
	psoDesc.SampleDesc = { 1,0 };
	psoDesc.SampleMask = UINT_MAX; // これを忘れると絵が出ない＆警告も出ないので注意.

	auto hr = m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_basePipeline));

	if (FAILED(hr))
	{
		throw std::runtime_error("CreateGraphicsPipelineState failed");
	}
}

void DefferdTest::prepare()
{
	createVertices();
	createIndices();
	compileShader();
	createRootSigunature();
	createPipeLine();
	createCbvHeap();
	createLight();
	createMaterial();
	createMvp();
}

void DefferdTest::cleanup()
{
}

void DefferdTest::MakeCommand(ComPtr<ID3D12GraphicsCommandList>& command)
{
	command->SetPipelineState(m_basePipeline.Get());
	command->SetGraphicsRootSignature(m_rootSignature.Get());

	command->RSSetViewports(1, &m_viewport);
	command->RSSetScissorRects(1, &m_scissorRect);

	ID3D12DescriptorHeap* heaps[] = { m_cbvHeap.Get() };
	command->SetDescriptorHeaps(_countof(heaps), heaps);

	command->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	command->IASetVertexBuffers(0,1,&m_vertexBufferView);
	command->IASetIndexBuffer(&m_indexBufferView);

	command->SetGraphicsRootDescriptorTable(0, m_lightBufferView);
	command->SetGraphicsRootDescriptorTable(1, m_mvpBufferViews[m_frameIndex]);
	command->SetGraphicsRootDescriptorTable(2, m_materialBufferView);

	command->DrawIndexedInstanced(m_indexCount,1,0,0,0);
	
}
