#pragma once
#include "GL.h"
#include <xstring>
typedef IShader<ID3D11VertexShader*, ID3DBlob> D3DVertexShaderBase;
typedef IShader<ID3D11PixelShader*, ID3DBlob> D3DPixelShader;

struct D3DVertexShader : public D3DVertexShaderBase
{
	ID3D11InputLayout* mInputLayout;

	D3DVertexShader(ID3D11VertexShader* _shader) : D3DVertexShaderBase(_shader), mInputLayout(nullptr)
	{}

	D3DVertexShader(ID3D11VertexShader* _shader, ID3DBlob* _blob) : D3DVertexShaderBase(_shader, _blob), mInputLayout(nullptr)
	{}

	void SetInputLayout(ID3D11InputLayout* _inputLayout)
	{
		//Assert(_mInputLayout == nullptr)
		mInputLayout = _inputLayout;
	}

	ID3D11InputLayout* GetInputLayout()
	{
		return mInputLayout;
	}
};
typedef IShaderProgram<D3DVertexShader, D3DPixelShader> D3DShaderProgram;

struct D3DBuffer
{
	ID3D11Buffer* buffer;

	D3DBuffer() : buffer(nullptr)
	{}

	D3DBuffer(ID3D11Buffer* _buffer) : buffer(_buffer)
	{}
};

#define USE_SAMPLER_STATE

struct D3DTexture
{
public:
	ID3D11Texture2D* mTexture;
	ID3D11ShaderResourceView* mShaderResourceView;
#if defined USE_SAMPLER_STATE
	ID3D11SamplerState* mSamplerState;
#endif
	unsigned int mBytePerRow;

public:
	D3DTexture() : mTexture(nullptr), mShaderResourceView(nullptr)
	{}

	D3DTexture(ID3D11Texture2D* _texture, ID3D11ShaderResourceView* _ShaderResourceView, ID3D11SamplerState* _SamplerState, unsigned int _bytePerRow) 
		: mTexture(_texture)
		, mShaderResourceView(_ShaderResourceView)
#if defined USE_SAMPLER_STATE
		, mSamplerState(_SamplerState)
#endif
		, mBytePerRow(_bytePerRow)
	{}
};

struct D3DRenderTarget : public D3DTexture
{
	ID3D11RenderTargetView* mRenderTargetView;
	
	ID3D11Texture2D* mDepthTexture;
	ID3D11DepthStencilView* mDepthStencilView;
	ID3D11DepthStencilState* mDepthStencilState;

	D3DRenderTarget() : D3DTexture()
	{
	}

	D3DRenderTarget(ID3D11Texture2D* _texture, ID3D11ShaderResourceView* _ShaderResourceView, ID3D11SamplerState* _SamplerState, ID3D11RenderTargetView* _renderTargetView, unsigned int _bytePerRow, ID3D11Texture2D* _depthTexture, ID3D11DepthStencilView* _DepthStencilView, ID3D11DepthStencilState* _DepthStencilState)
		: D3DTexture(_texture, _ShaderResourceView, _SamplerState, _bytePerRow)
		, mRenderTargetView(_renderTargetView)

		, mDepthTexture(_depthTexture)
		, mDepthStencilView(_DepthStencilView)
		, mDepthStencilState(_DepthStencilState)
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

	D3DRenderTarget* defaultRT;

	/**/
//	ID3D11RenderTargetView* rtv;
//	ID3D11Texture2D* depthStencilBuffer;
//	ID3D11DepthStencilView* depthStencilView;
//	ID3D11DepthStencilState* depthStencilState;
	/**/

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

// 	inline ID3D11RenderTargetView** _GetActualRenderTargetView()
// 	{
// 		//return (actualRenderTarget == nullptr) ? &defaultRT->mRenderTargetView : &actualRenderTarget->mRenderTargetView;
// 	}

	void _CreateTexture2D(unsigned int width, unsigned int height, unsigned int extra_bind_flags, void* _data, ID3D11Texture2D** texture, ID3D11ShaderResourceView** srv, ID3D11SamplerState** sampler)
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
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

		if (sampler != nullptr)
		{
			//=-=-=-=- Not req?
			D3D11_SAMPLER_DESC sDesc;
			ZeroMemory(&sDesc, sizeof(sDesc));
			sDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			sDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			sDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			sDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			sDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			sDesc.MinLOD = 0;
			sDesc.MaxLOD = 1;

			hr = dev->CreateSamplerState(&sDesc, sampler);
		}
	}

public:
	D3DRenderer() : swapchain(nullptr), dev(nullptr), devcon(nullptr), defaultRT(nullptr)
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
		if (bufferMask & COLOR_BUFFER)
		devcon->ClearRenderTargetView(actualRenderTarget->mRenderTargetView, mClearColor);
		if (bufferMask & (DEPTH_BUFFER | STENCIL_BUFFER))
			devcon->ClearDepthStencilView(actualRenderTarget->mDepthStencilView, (bufferMask & DEPTH_BUFFER ? D3D11_CLEAR_DEPTH : 0) | (bufferMask & STENCIL_BUFFER ? D3D11_CLEAR_STENCIL : 0), 1.0f, 0);
	}

	void SetViewport(int x, int y, int width, int height)
	{
		D3D11_VIEWPORT vp;
		vp.TopLeftX = float(x);
		vp.TopLeftY = float(y);
		vp.Width = float(width);
		vp.Height = float(height);
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
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

		devcon->IASetInputLayout(sprog->GetVS()->GetInputLayout());
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

		devcon->IASetPrimitiveTopology(PrimitiveTopology<pt>::DXTopology);
		devcon->DrawIndexed(nIndices, 0, 0);
	}

	template<PRIMITIVE_TOPOLOGY pt>
	void Render(unsigned int nVertex)
	{
		_SetBuffers();

		devcon->IASetPrimitiveTopology(PrimitiveTopology<pt>::DXTopology);
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

		ID3D11SamplerState* samplerState = nullptr;

		_CreateTexture2D(width, height, 0, nullptr, &texture, &srv, &samplerState);

		return new D3DTexture(texture, srv, samplerState/*nullptr*/, width * 4);
	}

	D3DRenderTarget* CreateRenderTarget2D(unsigned int width, unsigned int height)
	{
		ID3D11Texture2D* texture;
		ID3D11ShaderResourceView* srv;
		ID3D11RenderTargetView* rtv;

		ID3D11SamplerState* samplerState = nullptr;

		_CreateTexture2D(width, height, D3D11_BIND_RENDER_TARGET, nullptr, &texture, &srv, &samplerState);

		D3D11_RENDER_TARGET_VIEW_DESC rtDesc;
		rtDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtDesc.Texture2D.MipSlice = 0;
		HRESULT hr = dev->CreateRenderTargetView(texture, &rtDesc, &rtv);

		//Depth
		ID3D11Texture2D* depthTexture;
		ID3D11DepthStencilView* depthView;
		ID3D11DepthStencilState* depthState;
		
		D3D11_TEXTURE2D_DESC dsbDesc;
		ZeroMemory(&dsbDesc, sizeof(dsbDesc));
		dsbDesc.ArraySize = 1;
		dsbDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		dsbDesc.CPUAccessFlags = 0;
		dsbDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsbDesc.Width = width;
		dsbDesc.Height = height;
		dsbDesc.MipLevels = 1;
		dsbDesc.SampleDesc.Count = 1;
		dsbDesc.SampleDesc.Quality = 0;
		dsbDesc.Usage = D3D11_USAGE_DEFAULT;
		hr = dev->CreateTexture2D(&dsbDesc, nullptr, &depthTexture);

		hr = dev->CreateDepthStencilView(depthTexture, nullptr, &depthView);

		//Depth State
		D3D11_DEPTH_STENCIL_DESC dssDesc;
		dssDesc.DepthEnable = TRUE;
		dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dssDesc.DepthFunc = D3D11_COMPARISON_LESS;
		hr = dev->CreateDepthStencilState(&dssDesc, &depthState);

		return new D3DRenderTarget(texture, srv, samplerState, rtv, width * 4, depthTexture, depthView, depthState);
	}

	void SetRenderTarget(D3DRenderTarget* rt)
	{
		actualRenderTarget = rt == nullptr ? defaultRT : rt;
		devcon->OMSetRenderTargets(1, &actualRenderTarget->mRenderTargetView, actualRenderTarget->mDepthStencilView);
		devcon->OMSetDepthStencilState(actualRenderTarget->mDepthStencilState, 1);
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
		devcon->PSSetSamplers(0, 1, &d3dTexture->mSamplerState);
	}

	D3DVertexShader* CreateVertexShaderFromSourceFile(const char* fName)
	{
		ID3DBlob* errorBlob = nullptr;
		ID3DBlob* blob = nullptr;
		D3DVertexShader* ptr = nullptr;

		INFO_LOG("D3D - Load & Compile vertex shader: %s\n", fName);
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
			if (blob == nullptr)
			{
				ErrorExit("D3D VertexShader Log - file not found: %s\n", fName);
			}

			ID3D11VertexShader* shader;
			dev->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shader);
			ptr = new D3DVertexShader(shader, blob);
		}
		else
		{
			ErrorExit("D3D VertexShader Log: %s\n", errorBlob->GetBufferPointer());
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
		INFO_LOG("D3D - Load & Compile pixel shader: %s\n", fName);
		int fname_len = MultiByteToWideChar(CP_ACP, 0, fName, -1, fname, 0);
		fname = new wchar_t[fname_len];
		MultiByteToWideChar(CP_ACP, 0, fName, -1, fname, fname_len); // == fname_len
		std::wstring fileName = std::wstring(fname) + std::wstring(L".hlsl");
		delete[] fname;
		//ps_4_0_level_9_1
		D3DCompileFromFile(fileName.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &blob, &errorBlob);

		if (errorBlob == nullptr)
		{
			if (blob == nullptr)
			{
				ErrorExit("D3D PixelShader Log - file not found: %s\n", fName);
			}

			ID3D11PixelShader* shader;
			dev->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shader);
			ptr = new D3DPixelShader(shader, blob);
		}
		else
		{
			ErrorExit("D3D PixelShader Log: %s\n", errorBlob->GetBufferPointer());
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
	void DX_Defaults(IDXGISwapChain* _swapChain, ID3D11Device* _device, ID3D11DeviceContext* _devicecontext/*, ID3D11RenderTargetView* _rtv*/)
	{
		swapchain = _swapChain;
		dev = _device;
		devcon = _devicecontext;
		//rtv = _rtv;
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

		//D3D_FEATURE_LEVEL FeatureLevels = D3D_FEATURE_LEVEL_11_0;
		//D3D_FEATURE_LEVEL FeatureLevel;

		HRESULT hr = S_OK;
		hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, /*&FeatureLevels*/NULL, /*1*/0, D3D11_SDK_VERSION, &sd, &swapchain, &dev, /*&FeatureLevel*/NULL, &devcon);

		////Create Back buffer
		//Get a pointer to the back buffer
		ID3D11Texture2D* pBackBuffer;
		hr = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

		//Create a render-target view
		ID3D11RenderTargetView* rtv;
		dev->CreateRenderTargetView(pBackBuffer, NULL, &rtv);
		pBackBuffer->Release();

		//Create the depth buffer
		D3D11_TEXTURE2D_DESC dsbDesc;
		ZeroMemory(&dsbDesc, sizeof(dsbDesc));
		dsbDesc.ArraySize = 1;
		dsbDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		dsbDesc.CPUAccessFlags = 0;
		dsbDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsbDesc.Width = wnd->Width;
		dsbDesc.Height = wnd->Height;
		dsbDesc.MipLevels = 1;
		dsbDesc.SampleDesc.Count = 1;
		dsbDesc.SampleDesc.Quality = 0;
		dsbDesc.Usage = D3D11_USAGE_DEFAULT;
		ID3D11Texture2D* depthStencilBuffer;
		hr = dev->CreateTexture2D(&dsbDesc, nullptr, &depthStencilBuffer);

		ID3D11DepthStencilView* depthStencilView;
		hr = dev->CreateDepthStencilView(depthStencilBuffer, nullptr, &depthStencilView);

		//Depth State
		D3D11_DEPTH_STENCIL_DESC dssDesc;
		dssDesc.DepthEnable = TRUE;
		dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dssDesc.DepthFunc = D3D11_COMPARISON_LESS;
		ID3D11DepthStencilState* depthStencilState;
		hr = dev->CreateDepthStencilState(&dssDesc, &depthStencilState);

		//Bind the view
		defaultRT = new D3DRenderTarget(nullptr, nullptr, nullptr, rtv, wnd->Width * 4, depthStencilBuffer, depthStencilView, depthStencilState);
		SetRenderTarget(defaultRT);
	}

	unsigned char* GetScreenShot(unsigned int& _Width, unsigned int& _Height)
	{
		unsigned char* ptr = nullptr;
		ID3D11Texture2D* pBackBuffer;
		HRESULT hr = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

		if (pBackBuffer)
		{

			ID3D11Texture2D* pTemp;
			D3D11_TEXTURE2D_DESC descTemp;
			pBackBuffer->GetDesc(&descTemp);
			descTemp.BindFlags = 0;
			descTemp.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
			descTemp.Usage = D3D11_USAGE_STAGING;

			ptr = new unsigned char[descTemp.Width * descTemp.Height * 4];

			dev->CreateTexture2D(&descTemp, NULL, &pTemp);
			if (pTemp)
			{
				devcon->CopyResource(pTemp, pBackBuffer);
				D3D11_MAPPED_SUBRESOURCE res;
				unsigned int subresid = D3D11CalcSubresource(0, 0, 0);
				devcon->Map(pTemp, subresid, D3D11_MAP_READ_WRITE, 0, &res);

				const int width = descTemp.Width;
				const int widthD16 = (width + 15) & ~15;  //(((width + 15) >> 4) << 4);
				const int height = descTemp.Height;
				const int pitch = width * 4;
				const int pitchD16 = widthD16 * 4;
				const unsigned char* src = (unsigned char*)res.pData;
				unsigned char* dst = ptr;
				for (unsigned int i = 0; i < descTemp.Height; ++i)
				{
					memcpy(dst, src, pitch);
					src += pitchD16;
					dst += pitch;
				}
				_Width = width;
				_Height = height;
			}
		}
		
		return ptr;
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
		vs->SetInputLayout(ilay);
		delete[] inputElemDesc;
	}

	float* GetClearColor()
	{
		return mClearColor;
	}
};

template<>
struct MyTypes<D3D>
{
	typedef D3DRenderer Renderer;
	typedef D3DVertexShader VertexShader;
	typedef D3DPixelShader PixelShader;
	typedef D3DShaderProgram ShaderProgram;
	typedef D3DModel Model;
	typedef D3DBuffer VertexBuffer;
	typedef D3DBuffer IndexBuffer;
	typedef D3DBuffer ConstantBuffer;
	typedef D3DTexture Texture2D;
	typedef D3DRenderTarget RenderTarget;
};
