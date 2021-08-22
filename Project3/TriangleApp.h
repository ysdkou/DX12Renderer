#pragma once
#include "DX12App.h"
class TriangleApp:public DX12App
{
public:
	TriangleApp():DX12App() {};

	virtual void prepare()override;
	virtual void cleanup() override;
	virtual void MakeCommand(ComPtr<ID3D12GraphicsCommandList>& command);
private:
	ComPtr<ID3D12Resource1> createBuffer(UINT bufferSize, const void* initialData);

	ComPtr<ID3D12Resource1> m_vertexBuffer;
	ComPtr<ID3D12Resource1> m_indexBuffer;

	//
	
};

