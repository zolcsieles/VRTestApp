#define NULL ((void*)0)

typedef unsigned int HRESULT;
#define D3D11_BUFFER_DESC void
#define D3D11_SUBRESOURCE_DATA void
#define D3D11_RENDER_TARGET_VIEW_DESC void
#define ID3D11Resource void

class ID3D11Buffer
{
private:
	void* ptr;
	
public:
	ID3D11Buffer()
	{
		ptr = __coverity_alloc_nosize__();
	}
	
	void Release()
	{
		__coverity_free__(this);
	}
};

class ID3D11RenderTargetView
{
private:
	void* ptr;
	
public:
	ID3D11RenderTargetView()
	{
		ptr = __coverity_alloc_nosize__();
	}
	
	void Release()
	{
		__coverity_free__(this);
	}
};

class ID3D11Device
{
	HRESULT CreateBuffer(const D3D11_BUFFER_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Buffer **pBuffer)
	{
		*pBuffer = reinterpret_cast<ID3D11Buffer*>(__coverity_new__(sizeof(ID3D11Buffer)));
	}
	
	HRESULT CreateRenderTargetView(ID3D11Resource *pResource, const D3D11_RENDER_TARGET_VIEW_DESC *pDesc, ID3D11RenderTargetView **ppRTView)
	{
		ID3D11RenderTargetView* pRTView = reinterpret_cast<ID3D11RenderTargetView*>(__coverity_alloc__(sizeof(ID3D11RenderTargetView)));
		*ppRTView = pRTView;
	}
};

class ID3D11DeviceContext
{
	
};
