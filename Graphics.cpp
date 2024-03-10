#include "Graphics.h"
#include <sstream>
#include "Window.h"
#include "dxerr.h"
#include <d3dcompiler.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")
namespace wrl = Microsoft::WRL;

//graphics exception macros
#define GFX_EXCEPT_NOINFO(hrcall) Graphics::HrException(__LINE__,__FILE__,(hrcall))
#define GFX_THROW_NOINFO(hrcall) if( FAILED(hr = (hrcall)) ) throw GFX_EXCEPT_NOINFO(hr)

//graphics exception macros with dxgi info manager
#if !defined NDEBUG
#define GFX_EXCEPT(hrcall) Graphics::HrException(__LINE__,__FILE__,(hrcall),infoManager.GetMessages())
#define GFX_THROW_INFO(hrcall) infoManager.Set(); if( FAILED(hr = (hrcall)) ) throw GFX_EXCEPT(hr)
#define GFX_THROW_ONLY_INFO(call) infoManager.Set(); (call); auto msgs = infoManager.GetMessages(); if(!msgs.empty()) {throw Graphics::OnlyInfoException(__LINE__,__FILE__,msgs);}
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

	//Creating a descriptor for render target view
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	memset(&rtvDesc, 0, sizeof(rtvDesc));
	rtvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	//Creating the render target view
	GFX_THROW_INFO(pDevice->CreateRenderTargetView(pBackBuffer.Get(),&rtvDesc, &pRenderTarget)); // create render target view using the back buffer

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

void Graphics::DrawTriangle()
{
	HRESULT hr;

	//Creating the Vertex structure and setting the values
	struct Vertex {
		float x;
		float y;
	};
	const Vertex vertices[] = {
		{0.0f,0.5f},
		{0.5f,-0.5f},
		{-0.5f,-0.5f}
	};

	//Setting the primitive topology type for input assembler
	pContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Creating the blob for loading the bytecodes for input layout, vertex shader and pixel shader
	Microsoft::WRL::ComPtr<ID3DBlob> pBlobContents;

	//Loading the pixel shader bytecode
	D3DReadFileToBlob(L"PixelShader.cso", &pBlobContents);

	//Creating and Setting the Pixel Shader
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader;
	GFX_THROW_INFO(pDevice->CreatePixelShader(pBlobContents->GetBufferPointer(), pBlobContents->GetBufferSize(), nullptr, &pPixelShader));
	pContext->PSSetShader(pPixelShader.Get(), nullptr, 0u);

	//Vertex Buffer Descriptor
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = sizeof(vertices);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = 0u;
	bufferDesc.CPUAccessFlags = 0u;
	bufferDesc.MiscFlags = 0u;
	bufferDesc.StructureByteStride = sizeof(Vertex);

	//Vertex Buffer Subresource Data
	D3D11_SUBRESOURCE_DATA subData{};
	subData.pSysMem = vertices;

	//Creating and Setting the Vertex Buffer
	Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffers;
	GFX_THROW_INFO(pDevice->CreateBuffer(&bufferDesc, &subData, &pVertexBuffers));
	const UINT stride = sizeof(Vertex);
	const UINT offset = 0u;
	pContext->IASetVertexBuffers(0u, 1u, pVertexBuffers.GetAddressOf(), &stride, &offset);

	//Loading the vertex shader bytecode
	D3DReadFileToBlob(L"VertexShader.cso", &pBlobContents);

	//Creating and Setting the Vertex Shader
	Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader;
	GFX_THROW_INFO(pDevice->CreateVertexShader(pBlobContents->GetBufferPointer(), pBlobContents->GetBufferSize(), nullptr, &pVertexShader));
	pContext->VSSetShader(pVertexShader.Get(), nullptr, 0u);

	//Creating the descriptor for input layout
	const D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{
		"Position",
		0u,
		DXGI_FORMAT_R32G32_FLOAT,
		0u,
		0u,
		D3D11_INPUT_PER_VERTEX_DATA,
		0u
		}
	};

	//Creating the input layout
	Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;
	GFX_THROW_INFO(pDevice->CreateInputLayout(inputElementDesc, (UINT)std::size(inputElementDesc), pBlobContents->GetBufferPointer(), pBlobContents->GetBufferSize(), &pInputLayout));
	pContext->IASetInputLayout(pInputLayout.Get());

	//Creating viewport;
	D3D11_VIEWPORT viewPort;
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.Width = 800;
	viewPort.Height = 600;
	viewPort.MaxDepth = 1;
	viewPort.MinDepth = 0;
	pContext->RSSetViewports(1u, &viewPort);

	//Creating and setting render targets
	pContext->OMSetRenderTargets(1u, pRenderTarget.GetAddressOf(), nullptr);

	//Drawing
	GFX_THROW_ONLY_INFO(pContext->Draw((UINT)std::size(vertices), 0u));
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
	return "Graphics Hr Exception";
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

Graphics::OnlyInfoException::OnlyInfoException(int line, const char* file, std::vector<std::string> infoMsgs) noexcept
	:
	Exception(line, file)
{
	for (const auto& m : infoMsgs)
	{
		info += m;
		info.push_back('\n');
	}
	if (!info.empty())
	{
		info.pop_back();
	}
}

const char* Graphics::OnlyInfoException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl;
	if (!info.empty())
	{
		oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	}
	oss << GetOrigin();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Graphics::OnlyInfoException::GetType() const noexcept
{
	return "Graphics Only Info Exception";
}

std::string Graphics::OnlyInfoException::GetErrorInfo() const noexcept
{
	return info;
}
