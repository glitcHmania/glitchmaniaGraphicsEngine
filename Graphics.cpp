#include "Graphics.h"
#include <sstream>
#include "Window.h"
#include "dxerr.h"
#pragma comment(lib, "d3d11.lib")
namespace wrl = Microsoft::WRL;

//graphics exception macros
#define GFX_EXCEPT_NOINFO(hrcall) Graphics::HrException(__LINE__,__FILE__,(hrcall))
#define GFX_THROW_NOINFO(hrcall) if( FAILED(hr = (hrcall)) ) throw GFX_EXCEPT_NOINFO(hr)

//graphics exception macros with dxgi info manager
#if !defined NDEBUG
#define GFX_EXCEPT(hrcall) Graphics::HrException(__LINE__,__FILE__,(hrcall),infoManager.GetMessages())
#define GFX_THROW_INFO(hrcall) infoManager.Set(); if( FAILED(hr = (hrcall)) ) throw GFX_EXCEPT(hr)
#define GFX_DEVICE_REMOVED_EXCEPT(hrcall) Graphics::DeviceRemovedException(__LINE__,__FILE__,(hrcall),infoManager.GetMessages())
#else // release mode exception macros
#define GFX_EXCEPT(hrcall) Graphics::HrException(__LINE__,__FILE__,(hrcall))
#define GFX_THROW_INFO(hrcall) GFX_THROW_NOINFO(hrcall)
#define GFX_DEVICE_REMOVED_EXCEPT(hrcall) Graphics::DeviceRemovedException(__LINE__,__FILE__,(hrcall))
#endif



Graphics::Graphics(HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
	swapChainDesc.BufferDesc.Width = 0; // not specifying width and height. we'll use the window's dimensions
	swapChainDesc.BufferDesc.Height = 0;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // 32 bit color
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0; // not specifying refresh rate. we'll use the default
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED; // no scaling required because width and height are not specified
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;	
	swapChainDesc.SampleDesc.Count = 1; // no anti-aliasing
	swapChainDesc.SampleDesc.Quality = 0; 
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1; // one back buffer
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.Windowed = TRUE; // windowed mode
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; 
	swapChainDesc.Flags = 0;

	UINT createDeviceFlags = 0u;
#if !defined NDEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	//to check the results of d3d functions
	HRESULT hr;

	GFX_THROW_INFO(D3D11CreateDeviceAndSwapChain(
		nullptr, // using default adapter
		D3D_DRIVER_TYPE_HARDWARE, // using hardware graphics driver
		nullptr, // no software driver
		createDeviceFlags, // flags
		nullptr, // using default feature level array
		0, // use default feature level array size
		D3D11_SDK_VERSION, // use default SDK version
		&swapChainDesc, // pointer to swap chain description
		&pSwap, // pointer to swap chain interface
		&pDevice, // pointer to device interface
		nullptr, // no need to know supported feature level
		&pContext // pointer to device context interface
	));

	Microsoft::WRL::ComPtr<ID3D11Resource> pBackBuffer = nullptr; // pointer to back buffer
	GFX_THROW_INFO(pSwap->GetBuffer(0, __uuidof(ID3D11Resource), &pBackBuffer)); // fill the pointer with the back buffer
	GFX_THROW_INFO(pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &pRenderTarget)); // create render target view using the back buffer

}

void Graphics::EndFrame()
{
	HRESULT hr;
#if !defined NDEBUG
	infoManager.Set();
#endif
	if (FAILED(hr = pSwap->Present(2u, 0u))) // half a second vsync wait time and no flags
	{
		if (hr == DXGI_ERROR_DEVICE_REMOVED)
		{
			throw GFX_DEVICE_REMOVED_EXCEPT( pDevice->GetDeviceRemovedReason() );
		}
		else
		{
			GFX_EXCEPT(hr);
		}
	}
}

void Graphics::ClearBuffer(float red, float green, float blue) noexcept
{
	const float color[] = { red, green, blue, 1.0f };
	pContext->ClearRenderTargetView(pRenderTarget.Get(), color);
}

// Graphics exceptions
Graphics::HrException::HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs) noexcept
	:
	Exception(line, file),
	hr(hr)
{
	// join all info messages with newlines into single string
	for (const auto& m : infoMsgs)
	{
		info += m;
		info.push_back('\n');
	}
	// remove final newline if exists
	if (!info.empty())
	{
		info.pop_back();
	}
}

const char* Graphics::HrException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
		<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
		<< "[Error String] " << GetErrorString() << std::endl
		<< "[Description] " << GetErrorDescription() << std::endl;
	if (!info.empty())
	{
		oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	}
	oss << GetOrigin();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Graphics::HrException::GetType() const noexcept
{
	return "Graphics Exception";
}

HRESULT Graphics::HrException::GetErrorCode() const noexcept
{
	return hr;
}

std::string Graphics::HrException::GetErrorString() const noexcept
{
	return DXGetErrorString(hr);
}

std::string Graphics::HrException::GetErrorDescription() const noexcept
{
	char buf[512]; // Use with caution due to limitations mentioned above

	// Check if the buffer is large enough:
	if (sizeof(buf) < 512) {
		// Handle the insufficient buffer size, e.g., log or throw an error.
		return "Error: Buffer size is not sufficient.";
	}

	// Use FORMAT_MESSAGE_IGNORE_INSERTS to avoid truncation (but be aware of potential overflow):
	DWORD dwCharsWritten = FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		hr,
		0,
		buf,
		sizeof(buf),
		NULL
	);

	// Check if the buffer was large enough:
	if (dwCharsWritten >= sizeof(buf)) {
		// Handle potential truncation due to insufficient buffer size.
		return "Error: Error message truncated.";
	}

	return std::string(buf);
}

std::string Graphics::HrException::GetErrorInfo() const noexcept
{
	return info;
}


const char* Graphics::DeviceRemovedException::GetType() const noexcept
{
	return "Graphics Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)";
}
