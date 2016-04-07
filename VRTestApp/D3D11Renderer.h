#pragma once

#include <xstring>
typedef IShader<ID3D11VertexShader*, ID3DBlob> D3DVertexShader;
typedef IShader<ID3D11PixelShader*, ID3DBlob> D3DPixelShader;
typedef IShaderProgram<D3DVertexShader, D3DPixelShader> D3DShaderProgram;

struct D3DBuffer
{
	ID3D11Buffer* buffer;

	D3DBuffer() : buffer(nullptr)
	{}

	D3DBuffer(ID3D11Buffer* _buffer) : buffer(_buffer)
	{}
};

struct D3DTexture
{
	ID3D11Texture2D* mTexture;
	ID3D11ShaderResourceView* mShaderResourceView;
	ID3D11SamplerState* mSamplerState;

	unsigned int mBytePerRow;

public:
	D3DTexture() : mTexture(nullptr), mShaderResourceView(nullptr)
	{}

	D3DTexture(ID3D11Texture2D* _texture, ID3D11ShaderResourceView* _ShaderResourceView, ID3D11SamplerState* _SamplerState, unsigned int _bytePerRow) 
		: mTexture(_texture)
		, mShaderResourceView(_ShaderResourceView)
		, mSamplerState(_SamplerState)
		, mBytePerRow(_bytePerRow)
	{}
};

class D3DSampler
{
	ID3D11SamplerState* sampler;
};

class D3DModel : IModel<D3DBuffer>
{
private:
	friend class D3DRenderer;
	
public:
	D3DModel(Layout* layout) : IModel(layout)
	{
	}
};

class D3DRenderer
{
private:
	float mClearColor[4];
	D3DModel* actualModel;

protected:
	IDXGISwapChain* swapchain;
	ID3D11Device* dev;
	ID3D11DeviceContext* devcon;
	ID3D11RenderTargetView* rtv;

	ID3D11Buffer* _CreateAndUploadBuffer(int sizeOfBuffer, unsigned int bindFlags, const void* data)
	{
		ID3D11Buffer* temp;

		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = sizeOfBuffer;
		bufferDesc.BindFlags = bindFlags;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA bufferData;
		bufferData.pSysMem = data;
		bufferData.SysMemPitch = 0;
		bufferData.SysMemSlicePitch = 0;

		//Create and upload
		HRESULT hr;
		hr = dev->CreateBuffer(&bufferDesc, &bufferData, &temp);

		return temp;
	}

	ID3D11Buffer* _CreateBuffer(int sizeOfBuffer, unsigned int bindFlags)
	{
		ID3D11Buffer* temp;

		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = sizeOfBuffer;
		cbDesc.Usage = D3D11_USAGE_DEFAULT;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = 0;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;

		HRESULT hr = dev->CreateBuffer(&cbDesc, NULL, &temp);
		
		return temp;
	}

	void _SetBuffers()
	{
		if (actualModel->mIndex.buffer != nullptr)
			devcon->IASetIndexBuffer(actualModel->mIndex.buffer, FormatDescType<unsigned int>::DXGIFormats[0], 0);

		UINT offset = 0;
		for (unsigned int i = 0; i < actualModel->mLayout->GetSlotCount(); ++i)
		{
			UINT stride = actualModel->mLayout->GetSlotSize(i);
			devcon->IASetVertexBuffers(i, 1, &(actualModel->mSlots[i].buffer), &stride, &offset);
		}
	}

public:
	D3DRenderer() : swapchain(nullptr), dev(nullptr), devcon(nullptr), rtv(nullptr)
	{
	}

	void SetClearColor(float r, float g, float b, float a)
	{
		mClearColor[0] = r;
		mClearColor[1] = g;
		mClearColor[2] = b;
		mClearColor[3] = a;
	};
	
	void Clear(unsigned int bufferMask)
	{
		devcon->ClearRenderTargetView(rtv, mClearColor);
	}

	void SetViewport(int x, int y, int width, int height)
	{
		D3D11_VIEWPORT vp;
		vp.TopLeftX = float(x);
		vp.TopLeftY = float(y);
		vp.Width = float(width);
		vp.Height = float(height);
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 0.0f;
		devcon->RSSetViewports(1, &vp);
	}

	void SwapBuffers()
	{
		swapchain->Present(0, 0);
	}

	void ActivateProgram(D3DShaderProgram* sprog)
	{
		devcon->VSSetShader(*sprog->GetVS(), nullptr, 0);
		devcon->PSSetShader(*sprog->GetPS(), nullptr, 0);
	}

	void DeactivatePrograms()
	{

	}

	void BindModel(D3DModel* model)
	{
		actualModel = model;
	}

	D3DBuffer* CreateVertexBuffer(unsigned int slot, int nVertices, const void* vertData)
	{
		const int sizeOfBuffer = actualModel->mLayout->GetSlotSize(slot) * nVertices;
		actualModel->mSlots[slot].buffer = _CreateAndUploadBuffer(sizeOfBuffer, D3D11_BIND_VERTEX_BUFFER, vertData);
		return &actualModel->mSlots[slot];
	}

	D3DBuffer* CreateIndexBuffer(int nIndices, const void* indexData)
	{
		const int sizeOfBuffer = sizeof(unsigned int) * nIndices;
		actualModel->mIndex.buffer = _CreateAndUploadBuffer(sizeOfBuffer, D3D11_BIND_INDEX_BUFFER, indexData);
		return &actualModel->mIndex;
	}

	D3DBuffer* CreateConstantBuffer(int sizeOfBuffer)
	{
		return new D3DBuffer(_CreateBuffer(sizeOfBuffer, D3D11_BIND_CONSTANT_BUFFER));
	}

	void UpdateConstantBuffer(D3DBuffer* constBuffer, void* data)
	{
		devcon->UpdateSubresource(constBuffer->buffer, 0, nullptr, data, 0, 0);
	}

	void ActualizeConstantBuffer(D3DBuffer* constBuffer, D3DShaderProgram* shaderProgram, const char* blockName)
	{
		devcon->VSSetConstantBuffers(0, 1, &constBuffer->buffer);
	}
	template<PRIMITIVE_TOPOLOGY pt>
	void RenderIndexed(unsigned int nIndices)
	{
		_SetBuffers();

		devcon->IASetPrimitiveTopology(PrimitiveTopology<PT_TRIANGLE_LIST>::DXTopology);
		devcon->DrawIndexed(nIndices, 0, 0);
	}

	template<PRIMITIVE_TOPOLOGY pt>
	void Render(unsigned int nVertex)
	{
		_SetBuffers();

		devcon->IASetPrimitiveTopology(PrimitiveTopology<PT_TRIANGLE_LIST>::DXTopology);
		devcon->Draw(nVertex, 0);
	}

	void UnbindModels()
	{
		actualModel = nullptr;
	}

	D3DTexture* CreateTexture2D(unsigned int width, unsigned int height)
	{
		D3D11_TEXTURE2D_DESC desc;
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		/*
		Upload default:
		D3D11_SUBRESOURCE_DATA srdata;
		srdata.pSysMem = pointer;
		srdata.SysMemPitch = width*Bpp;
		srdata.SysMemSlicePitch = 0;
		//srdata is the 2. parameter of CreateTexture2D
		*/

		ID3D11Texture2D* texture;
		HRESULT hr = dev->CreateTexture2D(&desc, nullptr, &texture);

		//=-=-=-=-
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = desc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;

		ID3D11ShaderResourceView* shaderResourceView;
		hr = dev->CreateShaderResourceView(texture, &srvDesc, &shaderResourceView);

		//=-=-=-=-
		D3D11_SAMPLER_DESC sDesc;
		ZeroMemory(&sDesc, sizeof(sDesc));
		sDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sDesc.MinLOD = 0;
		sDesc.MaxLOD = D3D11_FLOAT32_MAX;

		ID3D11SamplerState* samplerState;
		hr = dev->CreateSamplerState(&sDesc, &samplerState);

		return new D3DTexture(texture, shaderResourceView, samplerState, width * 4);
	}

	void UploadTextureData(D3DTexture* d3dTexture, void* data)
	{
		devcon->UpdateSubresource(d3dTexture->mTexture, 0, nullptr, data, d3dTexture->mBytePerRow, 0);
	}

	void ActivateTexture(D3DTexture* d3dTexture)
	{
		devcon->PSSetShaderResources(0, 1, &d3dTexture->mShaderResourceView);
		devcon->PSSetSamplers(0, 1, &d3dTexture->mSamplerState);
	}

	D3DVertexShader* CreateVertexShaderFromSourceFile(const char* fName)
	{
		ID3DBlob* errorBlob = nullptr;
		ID3DBlob* blob = nullptr;
		D3DVertexShader* ptr = nullptr;

		wchar_t* fname = nullptr;
		int fname_len = MultiByteToWideChar(CP_ACP, 0, fName, -1, fname, 0);
		fname = new wchar_t[fname_len];
		MultiByteToWideChar(CP_ACP, 0, fName, -1, fname, fname_len); // == fname_len
		std::wstring fileName = std::wstring(fname) + std::wstring(L".hlsl");
		delete[] fname;
		//vs_4_0_level_9_1
		D3DCompileFromFile(fileName.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &blob, &errorBlob);

		if (errorBlob == nullptr)
		{
			ID3D11VertexShader* shader;
			dev->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shader);
			ptr = new D3DVertexShader(shader, blob);
		}
		else
		{
			Warning("Shader Log: %s\n", errorBlob->GetBufferPointer());
			errorBlob->Release();
		}

		return ptr;
	}

	D3DPixelShader* CreatePixelShaderFromSourceFile(const char* fName)
	{
		ID3DBlob* errorBlob = nullptr;
		ID3DBlob* blob = nullptr;
		D3DPixelShader* ptr = nullptr;

		wchar_t* fname = nullptr;
		int fname_len = MultiByteToWideChar(CP_ACP, 0, fName, -1, fname, 0);
		fname = new wchar_t[fname_len];
		MultiByteToWideChar(CP_ACP, 0, fName, -1, fname, fname_len); // == fname_len
		std::wstring fileName = std::wstring(fname) + std::wstring(L".hlsl");
		delete[] fname;
		//ps_4_0_level_9_1
		D3DCompileFromFile(fileName.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &blob, &errorBlob);

		if (errorBlob == nullptr)
		{
			ID3D11PixelShader* shader;
			dev->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shader);
			ptr = new D3DPixelShader(shader, blob);
		}
		else
		{
			errorBlob->Release();
		}

		return ptr;
	}

	D3DShaderProgram* CreateShaderProgram(D3DVertexShader* vs, D3DPixelShader* ps, Layout* layout)
	{
		D3DShaderProgram* prog = new D3DShaderProgram(vs, ps);
		AttachLayoutToProgram(layout, prog);
		return prog;
	}

	D3DModel* CreateModel(Layout* layout)
	{
		return new D3DModel(layout);
	}

	//for D3DRenderer
private:
	friend class DXAPP;

	void AttachLayoutToProgram(Layout* layout, D3DShaderProgram* prog)
	{
		AttachLayoutToVertexShader(layout, prog->GetVS());
	}

	void AttachLayoutToVertexShader(Layout* layout, D3DVertexShader* vs)
	{
		const int elemCount = layout->GetElemCount();
		ID3D11InputLayout* ilay;
		D3D11_INPUT_ELEMENT_DESC* inputElemDesc = new D3D11_INPUT_ELEMENT_DESC[elemCount];
		for (int i = 0; i < elemCount; ++i)
			layout->GetElem(i)->SetInputElementDesc(&inputElemDesc[i]);
		GetDevicePtr()->CreateInputLayout(inputElemDesc, elemCount, vs->GetBlob()->GetBufferPointer(), vs->GetBlob()->GetBufferSize(), &ilay);
		GetDeviceContextPtr()->IASetInputLayout(ilay);
		delete inputElemDesc;
	}

	IDXGISwapChain* GetSwapChainPtr() { return swapchain; }
	IDXGISwapChain** GetSwapChainPtrPtr() { return &swapchain; }

	ID3D11Device* GetDevicePtr() { return dev; }
	ID3D11Device** GetDevicePtrPtr() { return &dev; }

	ID3D11DeviceContext* GetDeviceContextPtr() { return devcon; }
	ID3D11DeviceContext** GetDeviceContextPtrPtr() { return &devcon; }

	ID3D11RenderTargetView* GetRenderTargetViewPtr() { return rtv; }
	ID3D11RenderTargetView** GetRenderTargetViewPtrPtr() { return &rtv; }
};
