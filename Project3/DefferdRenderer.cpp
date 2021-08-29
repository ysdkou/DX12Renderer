#include "DefferdRenderer.h"

void DefferdTest::createCbvHeap()
{
	//CBV
	//0,Light
	//1,2 MVP (frame count分)
	//3 Material
	MyDX12::DescriptorHeap::Builder builder;
	builder.setDevice(m_device.Get()).
		setHeapType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).
		setSize(CB_SRV_HEAP_SIZE).
		setFlagsShaderVisible();

	m_cbvsrvHeap = MyDX12::DescriptorHeap::Create(builder);
}
void DefferdTest::createLight()
{
	LightCb light = { {-1.0f,0.0f,0.0f},{1.0f,1.0f,1.0f} };

	MyDX12::ConstantBuffer::Builder builder;
	builder.seDevice(m_device.Get()).
		setSizeAndValueFromInstance(light);
	m_lightBuffer = MyDX12::ConstantBuffer::Create(builder);

	auto desc = m_lightBuffer->createViewDesc();
	m_cbvsrvHeap->createConstantBufferView(&desc, LIGHT_BUFFER_START_INDEX);

	m_lightBufferView = m_cbvsrvHeap->getGPUHandle(LIGHT_BUFFER_START_INDEX);

}
void DefferdTest::createModel()
{
	using namespace DirectX;

	ModelCb model{};
	XMStoreFloat4x4(&model.model, XMMatrixRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMConvertToRadians(45.0f)));

	MyDX12::ConstantBuffer::Builder builder;
	builder.seDevice(m_device.Get()).
		setSizeAndValueFromInstance(model);
	m_modelBuffer = MyDX12::ConstantBuffer::Create(builder);

	auto desc = m_modelBuffer->createViewDesc();
	m_cbvsrvHeap->createConstantBufferView(&desc, MODEL_BUFFER_START_INDEX);

	m_modelBufferView = m_cbvsrvHeap->getGPUHandle(MODEL_BUFFER_START_INDEX);
}
void DefferdTest::createMaterial() 
{

	MaterialCb material = { {0.9f,0.0f,0.0f},1.0f,1.0f,1.0f };

	MyDX12::ConstantBuffer::Builder builder;
	builder.seDevice(m_device.Get()).
		setSizeAndValueFromInstance(material);
	m_materialBuffer = MyDX12::ConstantBuffer::Create(builder);

	auto desc = m_materialBuffer->createViewDesc();

	m_cbvsrvHeap->createConstantBufferView(&desc, MATERIAL_BUFFER_START_INDEX);

	m_materialBufferView = m_cbvsrvHeap->getGPUHandle(MATERIAL_BUFFER_START_INDEX);
}
void DefferdTest::createViewProjection()
{
	m_viewProjectionBuffers.resize(FRAME_BUFFER_COUNT);
	m_viewProjectionBufferViews.resize(FRAME_BUFFER_COUNT);
	for (int i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		using namespace DirectX;

		//各行列のセット.
		ViewProjectionCb vp;
		auto eyePos = XMVectorSet(0.0f, 3.0f, -5.0f, 0.0f);
		XMStoreFloat3(&vp.cameraPos, eyePos);
		auto mtxView = XMMatrixLookAtLH(
			eyePos,
			XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
			XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
		);
		auto mtxProj = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), 480.0f / 360.0f, 0.1f, 100.0f);
		XMStoreFloat4x4(&vp.view, XMMatrixTranspose(mtxView));
		XMStoreFloat4x4(&vp.projection, XMMatrixTranspose(mtxProj));
		;
		MyDX12::ConstantBuffer::Builder builder;
		builder.seDevice(m_device.Get()).
			setSizeAndValueFromInstance(vp);
		
		m_viewProjectionBuffers.at(i) = MyDX12::ConstantBuffer::Create(builder);
;
		auto desc = m_viewProjectionBuffers.at(i)->createViewDesc();
		m_cbvsrvHeap->createConstantBufferView(&desc, VIEW_PROJECTION_BUFFER_START_INDEX + i);

		m_viewProjectionBufferViews.at(i) = m_cbvsrvHeap->getGPUHandle(VIEW_PROJECTION_BUFFER_START_INDEX + i);
	}

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

	MyDX12::VertexBuffer::Builder builder;
	builder.setDevice(m_device.Get()).
		setInitialValue(triangleVertices).
		setSize(sizeof(triangleVertices)).
		setStride(sizeof(Vertex));

	m_vertexBuffer = MyDX12::VertexBuffer::Create(builder);
	
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

	MyDX12::IndexBuffer::Builder builder;
	builder.setDevice(m_device.Get()).
		setSize(sizeof(indices)).
		setSizePerIndex(sizeof(uint32_t)).
		setInitialValue(indices);
	m_indexBuffer = MyDX12::IndexBuffer::Create(builder);

}

void DefferdTest::createRootSigunature()
{
	CD3DX12_DESCRIPTOR_RANGE light, mvp, material,model;
	light.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	mvp.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
    material.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);
	model.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 3);

	CD3DX12_ROOT_PARAMETER rootParams[4]{};

	rootParams[0].InitAsDescriptorTable(1, &light);
	rootParams[1].InitAsDescriptorTable(1, &mvp);
	rootParams[2].InitAsDescriptorTable(1, &material);
	rootParams[3].InitAsDescriptorTable(1, &model);


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
	hr = CompileShaderFromFile(L"GBufferPassVertex.hlsl", L"vs_6_0", m_vs, errBlob);
	if (FAILED(hr))
	{
		OutputDebugStringA((const char*)errBlob->GetBufferPointer());
		throw std::runtime_error("Shader Comple Failed");
	}
	hr = CompileShaderFromFile(L"GBufferPassPixel.hlsl", L"ps_6_0", m_ps, errBlob);
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

void DefferdTest::createGBuferRTV()
{
	//1.Albedo,roughness
	//2 Normal,metallic
	//3 ao

	MyDX12::DescriptorHeap::Builder builder;
	//DescriptorHeap生成
	builder.setDevice(m_device.Get()).
		setHeapType(D3D12_DESCRIPTOR_HEAP_TYPE_RTV).
		setSize(3).
		setFlagsNone();

	m_gBufferRtvHeap = MyDX12::DescriptorHeap::Create(builder);

	//GBufferTextureのResource生成
	CD3DX12_HEAP_PROPERTIES heapProperty(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC resouceDesc{};

	resouceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resouceDesc.Alignment = 0;
	resouceDesc.SampleDesc.Count = 1;
	resouceDesc.SampleDesc.Quality = 0;
	resouceDesc.MipLevels = 1;

	resouceDesc.DepthOrArraySize = 1;
	resouceDesc.Width = static_cast<UINT>(m_viewport.Width);
	resouceDesc.Height = static_cast<UINT>(m_viewport.Height);
	resouceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resouceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE clearVal;

	clearVal.Color[0] = m_clearColor[0];
	clearVal.Color[1] = m_clearColor[1];
	clearVal.Color[2] = m_clearColor[2];
	clearVal.Color[3] = m_clearColor[3];

	for (int i = 0; i < GBUFFER_SIZE; i++)
	{
		resouceDesc.Format = m_gBuffferFormat.at(i);
		clearVal.Format = m_gBuffferFormat.at(i);
		m_device->CreateCommittedResource(&heapProperty, D3D12_HEAP_FLAG_NONE, &resouceDesc, D3D12_RESOURCE_STATE_RENDER_TARGET, &clearVal, IID_PPV_ARGS(&m_gBufferTexture.at(i)));
	}

	//RenderTargetView生成
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};

	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;

	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	for (UINT i = 0; i < GBUFFER_SIZE; i++)
	{
		rtvDesc.Format = m_gBuffferFormat.at(i);
		m_gBufferRtvHeap->createRenderTargetView(m_gBufferTexture.at(i).Get(), &rtvDesc, i);
	}

	//ShaderResouceView生成

	D3D12_SHADER_RESOURCE_VIEW_DESC descSRV{};

	descSRV.Texture2D.MipLevels = resouceDesc.MipLevels;
	descSRV.Texture1D.MostDetailedMip = 0;
	descSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	descSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	
	for (int i = 0; i < GBUFFER_SIZE; i++)
	{
		descSRV.Format = m_gBuffferFormat.at(i);
		m_cbvsrvHeap->createShaderResouceView(m_gBufferTexture.at(i).Get(), &descSRV, GBUFFER_SRV_START_INDEX + i);
	}

}

void DefferdTest::createDepthSRV()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC descSRV{};

	descSRV.Texture2D.MipLevels = 1;
	descSRV.Texture2D.MostDetailedMip = 0;
	descSRV.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	descSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	descSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	
	m_cbvsrvHeap->createShaderResouceView(m_depthStencil.Get(), &descSRV, DEPTH_BUFFER_SRV_START_INDEX);
}

void DefferdTest::createGBufferPipeLine()
{
	// インプットレイアウト
	D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
	  { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA},
	};


	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};

	psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vs.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_ps.Get());

	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

	auto rasterizerDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.RasterizerState = rasterizerDesc;


	psoDesc.NumRenderTargets = GBUFFER_SIZE;
	psoDesc.RTVFormats[0] = m_gBuffferFormat[0];
	psoDesc.RTVFormats[1] = m_gBuffferFormat[1];
	psoDesc.RTVFormats[2] = m_gBuffferFormat[2];


	// デプスバッファのフォーマットを設定
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	auto depthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = depthStencilDesc;

	psoDesc.InputLayout = { inputElementDesc, _countof(inputElementDesc) };

	// ルートシグネチャのセット
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// マルチサンプル設定
	psoDesc.SampleDesc = { 1,0 };
	psoDesc.SampleMask = UINT_MAX; // これを忘れると絵が出ない＆警告も出ないので注意.

	auto hr = m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_gBufferPipeline));


}

void DefferdTest::prepare()
{
	createVertices();
	createIndices();
	compileShader();
	createRootSigunature();
	//createPipeLine();
	createGBufferPipeLine();
	createCbvHeap();
	createLight();
	createViewProjection();
	createMaterial();
	createModel();
	createGBuferRTV();
	createDepthSRV();
}

void DefferdTest::cleanup()
{
	for (int i = 0; i < GBUFFER_SIZE; i++)
	{

		
	}
}

void DefferdTest::MakeCommand(ComPtr<ID3D12GraphicsCommandList>& command)
{
	auto rtvHeapStart = m_gBufferRtvHeap->getCPUHandle(0);
	auto dsvHeapStart = m_heapDsv->GetCPUDescriptorHandleForHeapStart();
	
	for (int i = 0; i < GBUFFER_SIZE; i++)
	{
		command->ClearRenderTargetView(m_gBufferRtvHeap->getCPUHandle(i), m_clearColor.data(), 0, nullptr);
	}
	command->OMSetRenderTargets(GBUFFER_SIZE, &rtvHeapStart, true, &dsvHeapStart);

	command->SetPipelineState(m_gBufferPipeline.Get());
	command->SetGraphicsRootSignature(m_rootSignature.Get());

	command->RSSetViewports(1, &m_viewport);
	command->RSSetScissorRects(1, &m_scissorRect);

	ID3D12DescriptorHeap * heaps[] = { m_cbvsrvHeap->getRaw()};
	command->SetDescriptorHeaps(_countof(heaps), heaps);

	command->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	auto vertexBufferView = m_vertexBuffer->getBufferView();
	command->IASetVertexBuffers(0,1,&vertexBufferView);

	auto indexBufferView = m_indexBuffer->getBufferView();
	command->IASetIndexBuffer(&indexBufferView);

	command->SetGraphicsRootDescriptorTable(0, m_lightBufferView);
	command->SetGraphicsRootDescriptorTable(1, m_viewProjectionBufferViews[m_frameIndex]);
	command->SetGraphicsRootDescriptorTable(2, m_materialBufferView);
	command->SetGraphicsRootDescriptorTable(3,m_modelBufferView);


	command->DrawIndexedInstanced(m_indexBuffer->getCount(),1,0,0,0);
	
}
