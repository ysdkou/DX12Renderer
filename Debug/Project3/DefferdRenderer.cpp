#include "DefferdRenderer.h"
DirectX::XMFLOAT4X4 MatrixToFloat4x4(DirectX::XMMATRIX& mat)
{

	DirectX::XMFLOAT4X4 val;
	DirectX::XMStoreFloat4x4(&val, mat);
	return val;

}

void PrintF4x4(DirectX::XMFLOAT4X4 m,std::string_view name = std::string_view())
{
	using namespace DirectX;


	std::string top = std::to_string(m._11) + ',' + std::to_string(m._12) + ',' +  std::to_string(m._13) + ',' + std::to_string(m._14) + '\n';
	std::string second = std::to_string(m._21) + ',' + std::to_string(m._22) + ',' +std::to_string(m._23) + ',' +std::to_string(m._24) + '\n';
	std::string third = std::to_string(m._31) + ',' + std::to_string(m._32) + ',' +std::to_string(m._33) + ',' +std::to_string(m._34) + '\n';
	std::string fourth = std::to_string(m._41) + ',' + std::to_string(m._42) + ',' +std::to_string(m._43) + ',' +std::to_string(m._44) + '\n';

	std::string output = name.empty() ? top + second + third + fourth : std::string(name) + '\n' + top + second + third + fourth;
	OutputDebugStringA(output.c_str());

}
DirectX::XMMATRIX Float4x4ToMatrix(DirectX::XMFLOAT4X4& mat)
{
	return XMLoadFloat4x4(&mat);
}

void DefferdTest::createCbvHeap()
{
	//CBV SRV
	MyDX12::DescriptorHeap::Builder builder;
	builder.setDevice(m_device.Get()).
		setHeapType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).
		setSize(CB_SRV_HEAP_SIZE).
		setFlagsShaderVisible();

	m_cbvsrvHeap = MyDX12::DescriptorHeap::Create(builder);
}
void DefferdTest::createRTVHeap()
{
	MyDX12::DescriptorHeap::Builder builder;
	//DescriptorHeap生成
	builder.setDevice(m_device.Get()).
		setHeapType(D3D12_DESCRIPTOR_HEAP_TYPE_RTV).
		setSize(3).
		setFlagsNone();

	m_gBufferRtvHeap = MyDX12::DescriptorHeap::Create(builder);
}
void DefferdTest::createLight()
{
	LightCb light = { {-1.0f,1.0f,-1.0f},{5.0f,5.0f,5.0f} };

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

	m_modelBufferViews.resize(MODEL_BUFFER_SIZE);
	m_modelBuffers.resize(MODEL_BUFFER_SIZE);
	for (int row = 0; row < SPHERE_ROW; row++)
	{
		for (int col = 0; col < SPHERE_COL; col++)
		{
			ModelCb model{};

			auto transform = XMMatrixTranslation(
				static_cast<float>(col - (SPHERE_COL / 2)) * SPACE,
				static_cast<float>(row - (SPHERE_ROW / 2)) * SPACE,
					0);

			XMStoreFloat4x4(&model.model,XMMatrixTranspose(transform));

			transform.r[3] = XMVectorSet(0, 0, 0, 1);

			auto forNormal = XMMatrixTranspose(XMMatrixInverse(nullptr, transform));

			XMStoreFloat4x4(&model.forNormal, XMMatrixTranspose(forNormal));

			int i = SPHERE_ROW * row + col;
			MyDX12::ConstantBuffer::Builder builder;
			builder.seDevice(m_device.Get()).
				setSizeAndValueFromInstance(model);
			m_modelBuffers.at(i) = MyDX12::ConstantBuffer::Create(builder);

			auto desc = m_modelBuffers.at(i)->createViewDesc();
			m_cbvsrvHeap->createConstantBufferView(&desc, MODEL_BUFFER_START_INDEX + i);

			m_modelBufferViews.at(i) = m_cbvsrvHeap->getGPUHandle(MODEL_BUFFER_START_INDEX + i);
		}
	}
}


void DefferdTest::createMaterial() 
{
	m_materialBufferViews.resize(MATERIAL_BUFFER_SIZE);
	m_materialBuffers.resize(MATERIAL_BUFFER_SIZE);
	for (int row = 0; row < SPHERE_ROW; row++)
	{
		float metallic = static_cast<float>(row) / static_cast<float>(SPHERE_ROW);
		for (int col = 0; col < SPHERE_COL; col++)
		{
			
			float roughness = static_cast<float>(col) / static_cast<float>(SPHERE_COL);
			MaterialCb material = { {1.0f,0.0f,0.0f},metallic,roughness,1.0f };

			int i = SPHERE_ROW * row + col;
			MyDX12::ConstantBuffer::Builder builder;
			builder.seDevice(m_device.Get()).
				setSizeAndValueFromInstance(material);
			m_materialBuffers.at(i) = MyDX12::ConstantBuffer::Create(builder);

			auto desc = m_materialBuffers.at(i)->createViewDesc();

			m_cbvsrvHeap->createConstantBufferView(&desc, MATERIAL_BUFFER_START_INDEX + i);

			m_materialBufferViews.at(i) = m_cbvsrvHeap->getGPUHandle(MATERIAL_BUFFER_START_INDEX + i);
		}
	}
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
		auto eyePos = XMVectorSet(0.0f, 0.0f, -22.0f, 0.0f);
		XMStoreFloat3(&vp.cameraPos, eyePos);
		auto mtxView = XMMatrixLookAtLH(
			eyePos,
			XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
			XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
		);
		auto mtxProj = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), static_cast<float>(m_width) / static_cast<float>(m_height), 0.1f, 100.0f);
		/*XMStoreFloat4x4(&vp.view, XMMatrixTranspose(mtxView));
		XMStoreFloat4x4(&vp.projection, XMMatrixTranspose(mtxProj));*/

		XMFLOAT4X4 matScreen(2.0f / static_cast<float>(m_width), 0, 0, 0, 0, -2.0f / static_cast<float>(m_height), 0, 0, 0, 0, 1, 0, -1, 1, 0, 1);
		PrintF4x4(matScreen, "Screen");
		
		XMStoreFloat4x4(&vp.PV, XMMatrixTranspose(mtxView * mtxProj));
		
		auto detView = XMMatrixDeterminant(mtxView);
		auto detProj = XMMatrixDeterminant(mtxProj);
		auto invPV = XMMatrixInverse(&detProj, mtxProj) *XMMatrixInverse(&detView, mtxView);
		auto PV = mtxView * mtxProj;

		auto Identity = PV * invPV;

		XMFLOAT4X4 identityFF{};

		XMStoreFloat4x4(&identityFF, Identity);

		PrintF4x4(identityFF, "identity");
		XMStoreFloat4x4(&identityFF, PV);
		PrintF4x4(identityFF, "view - projection");
		XMStoreFloat4x4(&identityFF, invPV);
		PrintF4x4(identityFF, "int view - projection");



		auto fromDepth = XMMatrixTranspose(Float4x4ToMatrix(matScreen)*invPV) ;
		XMStoreFloat4x4(&vp.inversePV,fromDepth);

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


void DefferdTest::createBoxVertices()
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
		setSizeInBytes(sizeof(triangleVertices)).
		setStride(sizeof(Vertex));

	m_vertexBuffer = MyDX12::VertexBuffer::Create(builder);
	
}



void DefferdTest::createBoxIndices()
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
		setSizeInBytes(sizeof(indices)).
		setSizePerIndex(sizeof(uint32_t)).
		setInitialValue(indices);
	m_indexBuffer = MyDX12::IndexBuffer::Create(builder);

}

void DefferdTest::createSphereVertices()
{
	using namespace DirectX;
	auto radius = 1.0f;
	size_t segment = 50;
	size_t slice = 50;

	std::vector<Vertex> verts;
	verts.resize((segment + 1) * slice + 2);

	verts[0].position = XMFLOAT3(0,radius, 0);
	for (size_t lat = 0; lat < slice; lat++)
	{
		float a1 = XM_PI* (float)(lat + 1) / (slice+ 1);
		float sin1 = sinf(a1);
		float cos1 = cosf(a1);

		for (int lon = 0; lon <= segment; lon++)
		{
			float a2 = XM_2PI* (float)(lon == segment? 0 : lon) / segment;
			float sin2 = sinf(a2);
			float cos2 = cosf(a2);

			verts[lon + lat * (segment + 1) + 1].position = XMFLOAT3(sin1 * cos2 *radius, cos1 *radius, sin1 * sin2 *radius);
		}
	}
	verts[verts.size() - 1].position = XMFLOAT3(0, radius, 0);

	for (int n = 0; n < verts.size(); n++)
	{
		XMStoreFloat3(&verts[n].normal, XMVector3Normalize(XMVectorSet(verts[n].position.x, verts[n].position.y, verts[n].position.z, 0)));
	}
	
	MyDX12::VertexBuffer::Builder builder;
	builder.setDevice(m_device.Get()).
		setInitialValue(verts.data()).
		setSizeInBytes(verts.size()*sizeof(Vertex)).
		setStride(sizeof(Vertex));

	m_vertexBuffer = MyDX12::VertexBuffer::Create(builder);

	m_vertSize = verts.size();
}

void DefferdTest::createSphereIndices()
{
	auto radius = 1.0f;
	size_t segment = 50;
	size_t slice = 50;

	int nbFaces = m_vertSize;
	int nbTriangles = nbFaces * 2;
	int nbIndexes = nbTriangles * 3;
	std::vector< uint32_t>  triangles(nbIndexes);
	//int* triangles = new int[nbIndexes];


	int i = 0;
	for (int lon = 0; lon < segment; lon++)
	{
		triangles[i++] = lon + 2;
		triangles[i++] = lon + 1;
		triangles[i++] = 0;
	}

	//Middle
	for (int lat = 0; lat < slice - 1; lat++)
	{
		for (int lon = 0; lon < segment; lon++)
		{
			int current = lon + lat * (segment + 1) + 1;
			int next = current + segment + 1;

			triangles[i++] = current;
			triangles[i++] = current + 1;
			triangles[i++] = next + 1;

			triangles[i++] = current;
			triangles[i++] = next + 1;
			triangles[i++] = next;
		}
	}

	//Bottom Cap
	for (int lon = 0; lon < segment; lon++)
	{
		triangles[i++] = m_vertSize - 1;
		triangles[i++] = m_vertSize - (lon + 2) - 1;
		triangles[i++] = m_vertSize - (lon + 1) - 1;
	}

	MyDX12::IndexBuffer::Builder builder;
	builder.setDevice(m_device.Get()).
		setSizeInBytes(triangles.size()* sizeof(uint32_t)).
		setSizePerIndex(sizeof(uint32_t)).
		setInitialValue(triangles.data());
	m_indexBuffer = MyDX12::IndexBuffer::Create(builder);


}

void DefferdTest::createScreenQuad()
{
	using namespace DirectX;
	struct QuadVertex
	{
		XMFLOAT4 position;
		XMFLOAT2 texcoord;
	};

	QuadVertex quadVerts[] =
	{
		{ { -1.0f,1.0f, 0.0f,1.0f },{ 0.0f,0.0f } },
		{ { 1.0f, 1.0f, 0.0f,1.0f }, {1.0f,0.0f } },
		{ { -1.0f, -1.0f, 0.0f,1.0f },{ 0.0f,1.0f } },
		{ { 1.0f, -1.0f, 0.0f,1.0f },{ 1.0f,1.0f } }
	};

	MyDX12::VertexBuffer::Builder builder;
	builder.setDevice(m_device.Get()).
		setInitialValue(quadVerts).
		setSizeInBytes(sizeof(quadVerts)).
		setStride(sizeof(QuadVertex));

	m_screenQuadVertex = MyDX12::VertexBuffer::Create(builder);


}

void DefferdTest::createRootSigunature()
{
	CD3DX12_DESCRIPTOR_RANGE light, mvp, material, model, gBuffer;
	light.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	mvp.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
    material.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);
	model.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 3);
	gBuffer.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4,0);



	CD3DX12_ROOT_PARAMETER rootParams[5]{};

	rootParams[0].InitAsDescriptorTable(1, &light);
	rootParams[1].InitAsDescriptorTable(1, &mvp);
	rootParams[2].InitAsDescriptorTable(1, &material);
	rootParams[3].InitAsDescriptorTable(1, &model);
	rootParams[4].InitAsDescriptorTable(1, &gBuffer);

	CD3DX12_ROOT_SIGNATURE_DESC desc;

	desc.Init(_countof(rootParams), rootParams, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	CD3DX12_STATIC_SAMPLER_DESC staticSamplers[1]{};
	staticSamplers[0].Init(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);

	desc.NumStaticSamplers = 1;
	desc.pStaticSamplers = staticSamplers;

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
	hr = CompileShaderFromFile(L"GBufferPassVertex.hlsl", L"vs_6_0", m_gBufferVS, errBlob);
	if (FAILED(hr))
	{
		OutputDebugStringA((const char*)errBlob->GetBufferPointer());
		throw std::runtime_error("Shader Comple Failed");
	}
	hr = CompileShaderFromFile(L"GBufferPassPixel.hlsl", L"ps_6_0", m_gBufferPS, errBlob);
	if (FAILED(hr))
	{
		OutputDebugStringA((const char*)errBlob->GetBufferPointer());
		throw std::runtime_error("Shader Comple Failed");
	}

	hr = CompileShaderFromFile(L"LightPassVertex.hlsl", L"vs_6_0", m_lightPassVS ,errBlob);
	if (FAILED(hr))
	{
		OutputDebugStringA((const char*)errBlob->GetBufferPointer());
		throw std::runtime_error("Shader Comple Failed");
	}
	hr = CompileShaderFromFile(L"LightPassPixel.hlsl", L"ps_6_0", m_lightPassPS, errBlob);
	if (FAILED(hr))
	{
		OutputDebugStringA((const char*)errBlob->GetBufferPointer());
		throw std::runtime_error("Shader Comple Failed");
	}
}



void DefferdTest::createGBuferRTV()
{
	//1.Albedo,roughness
	//2 Normal,metallic
	//3 ao

	m_gBufferTexture.resize(GBUFFER_SIZE);

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
	  { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA},
	};


	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};

	psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_gBufferVS.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_gBufferPS.Get());

	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);


	psoDesc.NumRenderTargets = GBUFFER_SIZE;
	psoDesc.RTVFormats[0] = m_gBuffferFormat[0];
	psoDesc.RTVFormats[1] = m_gBuffferFormat[1];
	psoDesc.RTVFormats[2] = m_gBuffferFormat[2];


	// デプスバッファのフォーマットを設定
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

	psoDesc.InputLayout = { inputElementDesc, _countof(inputElementDesc) };

	// ルートシグネチャのセット
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// マルチサンプル設定
	psoDesc.SampleDesc = { 1,0 };
	psoDesc.SampleMask = UINT_MAX; // これを忘れると絵が出ない＆警告も出ないので注意.

	auto hr = m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_gBufferPipeline));


}

void DefferdTest::createLightPassPipeLine()
{
	D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }	
	};
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};

	psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_lightPassVS.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_lightPassPS.Get());

	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.DepthClipEnable = FALSE;

	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE;

	psoDesc.InputLayout = { inputElementDesc, _countof(inputElementDesc) };
  
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	psoDesc.SampleDesc = { 1,0 };
	psoDesc.SampleMask = UINT_MAX; // これを忘れると絵が出ない＆警告も出ないので注意.

	auto hr = m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_lightPassPipeline));

}

void DefferdTest::makeCommandGBufferPass(ComPtr<ID3D12GraphicsCommandList>& command)
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

	ID3D12DescriptorHeap* heaps[] = { m_cbvsrvHeap->getRaw() };
	command->SetDescriptorHeaps(_countof(heaps), heaps);

	command->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	auto vertexBufferView = m_vertexBuffer->getBufferView();
	command->IASetVertexBuffers(0, 1, &vertexBufferView);

	auto indexBufferView = m_indexBuffer->getBufferView();
	command->IASetIndexBuffer(&indexBufferView);

	for (int i = 0; i < MATERIAL_BUFFER_SIZE; i++)
	{
		command->SetGraphicsRootDescriptorTable(0, m_lightBufferView);
		command->SetGraphicsRootDescriptorTable(1, m_viewProjectionBufferViews[m_frameIndex]);
		command->SetGraphicsRootDescriptorTable(2, m_materialBufferViews.at(i));
		command->SetGraphicsRootDescriptorTable(3, m_modelBufferViews.at(i));
		command->SetGraphicsRootDescriptorTable(4, m_cbvsrvHeap->getGPUHandle(GBUFFER_SRV_START_INDEX));

		command->DrawIndexedInstanced(m_indexBuffer->getCount(), 1, 0, 0, 0);
	}
}


void DefferdTest::makeCommandLightPass(ComPtr<ID3D12GraphicsCommandList>& command)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(m_heapRtv->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	command->OMSetRenderTargets(1, &rtv, FALSE,nullptr);

	command->SetPipelineState(m_lightPassPipeline.Get());
	command->SetGraphicsRootSignature(m_rootSignature.Get());

	command->RSSetViewports(1, &m_viewport);
	command->RSSetScissorRects(1, &m_scissorRect);

	for (int i = 0; i < GBUFFER_SIZE; i++)
	{
		auto barrierToSRV = CD3DX12_RESOURCE_BARRIER::Transition(m_gBufferTexture.at(i).Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
		command->ResourceBarrier(1, &barrierToSRV);
	}

	auto barrierToSRVDepth = CD3DX12_RESOURCE_BARRIER::Transition(m_depthStencil.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);
	command->ResourceBarrier(1, &barrierToSRVDepth);

	command->SetGraphicsRootDescriptorTable(0, m_lightBufferView);
	command->SetGraphicsRootDescriptorTable(1, m_viewProjectionBufferViews[m_frameIndex]);
	command->SetGraphicsRootDescriptorTable(4, m_cbvsrvHeap->getGPUHandle(GBUFFER_SRV_START_INDEX));

	command->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	auto vertexBufferView = m_screenQuadVertex->getBufferView();
	command->IASetVertexBuffers(0, 1, &vertexBufferView);
	command->DrawInstanced(4, 1, 0, 0);

	for (int i = 0; i < GBUFFER_SIZE; i++)
	{
		auto barrierToRTV = CD3DX12_RESOURCE_BARRIER::Transition(m_gBufferTexture.at(i).Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
		command->ResourceBarrier(1, &barrierToRTV);
	}

	auto barrierToRTVDepth = CD3DX12_RESOURCE_BARRIER::Transition(m_depthStencil.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	command->ResourceBarrier(1, &barrierToRTVDepth);
}

void DefferdTest::prepare()
{
	createSphereVertices();
	createSphereIndices();
	createScreenQuad();
	compileShader();
	createRootSigunature();
	createGBufferPipeLine();
	createLightPassPipeLine();
	createCbvHeap();
	createRTVHeap();
	createLight();
	createViewProjection();
	createMaterial();
	createModel();
	createGBuferRTV();
	createDepthSRV();
}

void DefferdTest::cleanup()
{
	auto index = m_swapchain->GetCurrentBackBufferIndex();
	auto fence = m_frameFences[index];
	auto value = ++m_frameFenceValue[index];
	m_commandQueue->Signal(fence.Get(), value);
	fence->SetEventOnCompletion(value, m_fenceWaitEvent);
	WaitForSingleObject(m_fenceWaitEvent, GPU_WAIT_TIME_OUT);
}

void DefferdTest::MakeCommand(ComPtr<ID3D12GraphicsCommandList>& command)
{

	makeCommandGBufferPass(command);
	makeCommandLightPass(command);
}
