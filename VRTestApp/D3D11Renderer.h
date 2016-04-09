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
public:
	ID3D11Texture2D* mTexture;
	ID3D11ShaderResourceView* mShaderResourceView;
	//ID3D11SamplerState* mSamplerState;

	unsigned int mBytePerRow;

public:
	D3DTexture() : mTexture(nullptr), mShaderResourceView(nullptr)
	{}

	D3DTexture(ID3D11Texture2D* _texture, ID3D11ShaderResourceView* _ShaderResourceView, ID3D11SamplerState* _SamplerState, unsigned int _bytePerRow) 
		: mTexture(_texture)
		, mShaderResourceView(_ShaderResourceView)
		//, mSamplerState(_SamplerState)
		, mBytePerRow(_bytePerRow)
	{}
};

struct D3DRenderTarget : public D3DTexture
{
	ID3D11RenderTargetView* mRenderTargetView;
	//ID3D11DepthStencilView* mDepthStencilView;

	D3DRenderTarget() : D3DTexture()
	{
	}

	D3DRenderTarget(ID3D11Texture2D* _texture, ID3D11ShaderResourceView* _ShaderResourceView, ID3D11SamplerState* _SamplerState, ID3D11RenderTargetView* _renderTargetView, unsigned int _bytePerRow)
		: D3DTexture(_texture, _ShaderResourceView, _SamplerState, _bytePerRow)
		, mRenderTargetView(_renderTargetView)
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
	D3DRenderTarget* actualRenderTarget;

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

	inline ID3D11RenderTargetView** _GetActualRenderTargetView()
	{
		return (actualRenderTarget == nullptr) ? &rtv : &actualRenderTarget->mRenderTargetView;
	}

	void _CreateTexture2D(unsigned int width, unsigned int height, unsigned int extra_bind_flags, void* _data, ID3D11Texture2D** texture, ID3D11ShaderResourceView** srv)
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
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | extra_bind_flags;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		HRESULT hr;
		if (_data != nullptr)
		{
			D3D11_SUBRESOURCE_DATA srdata;
			srdata.pSysMem = _data;
			srdata.SysMemPitch = width * 4;
			srdata.SysMemSlicePitch = 0;
			hr = dev->CreateTexture2D(&desc, &srdata, texture);
		}
		else
		{
			hr = dev->CreateTexture2D(&desc, nullptr, texture);
		}

		//Shader Resource View
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = desc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;

		hr = dev->CreateShaderResourceView(*texture, &srvDesc, srv);
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
		devcon->ClearRenderTargetView(*_GetActualRenderTargetView(), mClearColor);
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
		ID3D11Texture2D* texture;
		ID3D11ShaderResourceView* srv;
		_CreateTexture2D(width, height, 0, nullptr, &texture, &srv);

		//=-=-=-=- Not req?
		/*D3D11_SAMPLER_DESC sDesc;
		ZeroMemory(&sDesc, sizeof(sDesc));
		sDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sDesc.MinLOD = 0;
		sDesc.MaxLOD = D3D11_FLOAT32_MAX;

		ID3D11SamplerState* samplerState;
		hr = dev->CreateSamplerState(&sDesc, &samplerState);*/

		return new D3DTexture(texture, srv, /*samplerState*/nullptr, width * 4);
	}

	D3DRenderTarget* CreateRenderTarget2D(unsigned int width, unsigned int height)
	{
		ID3D11Texture2D* texture;
		ID3D11ShaderResourceView* srv;
		ID3D11RenderTargetView* rtv;
		_CreateTexture2D(width, height, D3D11_BIND_RENDER_TARGET, nullptr, &texture, &srv);

		D3D11_RENDER_TARGET_VIEW_DESC rtDesc;
		rtDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtDesc.Texture2D.MipSlice = 0;
		HRESULT hr = dev->CreateRenderTargetView(texture, &rtDesc, &rtv);

		return new D3DRenderTarget(texture, srv, nullptr, rtv, width * 4);
	}

	void SetRenderTarget(D3DRenderTarget* rt)
	{
		actualRenderTarget = rt;
		devcon->OMSetRenderTargets(1, _GetActualRenderTargetView(), NULL);
	}

	void SetDefaultRenderTarget(D3DRenderTarget* rt)
	{
	}

	void UploadTextureData(D3DTexture* d3dTexture, void* data)
	{
		devcon->UpdateSubresource(d3dTexture->mTexture, 0, nullptr, data, d3dTexture->mBytePerRow, 0);
	}

	void ActivateTexture(D3DTexture* d3dTexture)
	{
		devcon->PSSetShaderResources(0, 1, &d3dTexture->mShaderResourceView);
		//devcon->PSSetSamplers(0, 1, &d3dTexture->mSamplerState);
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
	void DX_Defaults(IDXGISwapChain* _swapChain, ID3D11Device* _device, ID3D11DeviceContext* _devicecontext, ID3D11RenderTargetView* _rtv)
	{
		swapchain = _swapChain;
		dev = _device;
		devcon = _devicecontext;
		rtv = _rtv;
	}

	void Init(Window* wnd)
	{
		SDL_SysWMinfo wmInfo;
		SDL_VERSION(&wmInfo.version);
		SDL_GetWindowWMInfo(wnd->window, &wmInfo);

		HWND hWnd = wmInfo.info.win.window;
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));

		sd.BufferCount = 1;
		sd.BufferDesc.Width = wnd->Width;
		sd.BufferDesc.Height = wnd->Height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60; //0, ha no vsync
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		D3D_FEATURE_LEVEL FeatureLevels = D3D_FEATURE_LEVEL_11_0;
		D3D_FEATURE_LEVEL FeatureLevel;

		HRESULT hr = S_OK;
		hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &FeatureLevels, 1, D3D11_SDK_VERSION, &sd, &swapchain, &dev, &FeatureLevel, &devcon);

		////Create Back buffer

		//Get a pointer to the back buffer
		ID3D11Texture2D* pBackBuffer;
		hr = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

		//Create a render-target view
		dev->CreateRenderTargetView(pBackBuffer, NULL, &rtv);
		pBackBuffer->Release();

		//Bind the view
		SetRenderTarget(nullptr);
	}

private:
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
		dev->CreateInputLayout(inputElemDesc, elemCount, vs->GetBlob()->GetBufferPointer(), vs->GetBlob()->GetBufferSize(), &ilay);
		devcon->IASetInputLayout(ilay);
		delete inputElemDesc;
	}

	float* GetClearColor()
	{
		return mClearColor;
	}
};
