#include "Graphics.h"
#include <sstream>
#include "Window.h"
#include "GraphicsExceptionMacros.h"
#include <d3dcompiler.h>
#include <DirectXMath.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")
namespace wrl = Microsoft::WRL;





Graphics::Graphics(HWND hWnd)
{
	//To check the results of d3d functions
	HRESULT hr;

	//Creating the descriptor for swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
	swapChainDesc.BufferDesc.Width = 0; // not specifying width and height. we'll use the window's dimensions
	swapChainDesc.BufferDesc.Height = 0;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT; // 32 bit color
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0; // not specifying refresh rate. we'll use the default
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED; // no scaling required because width and height are not specified
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;	
	swapChainDesc.SampleDesc.Count = 1; // no anti-aliasing
	swapChainDesc.SampleDesc.Quality = 0; 
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1; // one back buffer
	swapChainDesc.OutputWindow = (HWND)6969; //hWnd;
	swapChainDesc.Windowed = TRUE; // windowed mode
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; 
	swapChainDesc.Flags = 0;

	UINT createDeviceFlags = 0u;
#if !defined NDEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	//Creating device and swap chain
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

	//Getting a pointer for back buffer
	Microsoft::WRL::ComPtr<ID3D11Resource> pBackBuffer = nullptr; // pointer to back buffer
	GFX_THROW_INFO(pSwap->GetBuffer(0, __uuidof(ID3D11Resource), &pBackBuffer)); // fill the pointer with the back buffer

	//Creating render target view descriptor
	//D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	//rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	//rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

	//Creating the render target view
	GFX_THROW_INFO(pDevice->CreateRenderTargetView(pBackBuffer.Get(),nullptr, &pRenderTargetView)); // create render target view using the back buffer

	//Creating and setting depth stencil state
	D3D11_DEPTH_STENCIL_DESC dsDesc={};
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pDepthStencilState;
	GFX_THROW_INFO(pDevice->CreateDepthStencilState(&dsDesc, &pDepthStencilState));
	pContext->OMSetDepthStencilState(pDepthStencilState.Get(), 1u);

	//Creating and setting depth stencil texture
	Microsoft::WRL::ComPtr<ID3D11Texture2D> pDepthStencilTexture;
	D3D11_TEXTURE2D_DESC descDepth={};
	descDepth.Width = (UINT)800.0f;
	descDepth.Height = (UINT)600.0f;
	descDepth.MipLevels = 1u;
	descDepth.ArraySize = 1u;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1u;
	descDepth.SampleDesc.Quality = 0u;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	GFX_THROW_INFO(pDevice->CreateTexture2D(&descDepth, nullptr, &pDepthStencilTexture));

	//Creating depth stencil view desc
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = DXGI_FORMAT_D32_FLOAT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0u;

	//Creating and setting the depth stencil view
	GFX_THROW_INFO(pDevice->CreateDepthStencilView(pDepthStencilTexture.Get(), &descDSV, &pDepthStencilView));

	//Setting the render target view and depth stencil view
	pContext->OMSetRenderTargets(1u, pRenderTargetView.GetAddressOf(), pDepthStencilView.Get());

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
	pContext->ClearRenderTargetView(pRenderTargetView.Get(), color);
	pContext->ClearDepthStencilView(pDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);
}

void Graphics::DrawTriangle(float posX, float posY, float posZ, float rotX, float rotY)
{
	HRESULT hr;

	//Setting the primitive topology type for input assembler
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Creating the vertex structure and setting the values
	struct Vertex {
		float x;
		float y;
		float z;
	};
	const Vertex vertices[] = {
	//Front face
	{0.0f	,0.8f	,-0.12f},//0
	{0.192f ,0.192f	,-0.12f},//1
	{0.8f	,0.192f	,-0.12f},//2
	{0.312f ,-0.208f,-0.12f},//3
	{0.488f ,-0.8f	,-0.12f},//4
	{0.0f	,-0.424f,-0.12f},//5
	{-0.488f,-0.8f	,-0.12f},//6
	{-0.312f,-0.208f,-0.12f},//7
	{-0.8f	,0.192f	,-0.12f},//8
	{-0.192f,0.192f	,-0.12f},//9

	//Back face
	{0.0f	,0.8f	,0.12f},//10
	{0.192f ,0.192f	,0.12f},//11
	{0.8f	,0.192f	,0.12f},//12
	{0.312f ,-0.208f,0.12f},//13
	{0.488f ,-0.8f	,0.12f},//14
	{0.0f	,-0.424f,0.12f},//15
	{-0.488f,-0.8f	,0.12f},//16
	{-0.312f,-0.208f,0.12f},//17
	{-0.8f	,0.192f	,0.12f},//18
	{-0.192f,0.192f	,0.12f} //19
	};

	//Vertex buffer subresource data
	D3D11_SUBRESOURCE_DATA vertexSD{};
	vertexSD.pSysMem = vertices;

	//Vertex buffer descriptor
	D3D11_BUFFER_DESC vertexBD{};
	vertexBD.ByteWidth = sizeof(vertices);
	vertexBD.Usage = D3D11_USAGE_DEFAULT;
	vertexBD.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBD.CPUAccessFlags = 0u;
	vertexBD.MiscFlags = 0u;
	vertexBD.StructureByteStride = sizeof(Vertex);

	//Creating and setting the vertex buffer
	Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffers;
	GFX_THROW_INFO(pDevice->CreateBuffer(&vertexBD, &vertexSD, &pVertexBuffers));
	const UINT stride = sizeof(Vertex);
	const UINT offset = 0u;
	pContext->IASetVertexBuffers(0u, 1u, pVertexBuffers.GetAddressOf(), &stride, &offset);

	//Setting the index values
	const unsigned short indices[] = {
	0,1,9,//front face
	1,2,3,
	3,4,5,
	5,6,7,
	7,8,9,
	9,5,7,
	1,3,5,
	9,1,5,
	11,10,19,//back face
	13,12,11,
	15,14,13,
	17,16,15,
	19,18,17,
	19,17,15,
	11,15,13,
	19,15,11,
	19,10,0,	19,0,9, // sides
	11,0,10,	11,1,0,
	11,12,1,	2,1,12,
	3,2,12,		3,12,13,
	4,3,13,		13,14,4,
	14,5,4,		14,15,5,
	6,5,15,		6,15,16,
	6,16,7,		16,17,7,
	8,7,17,		17,18,8,
	19,9,8,		18,19,8
	};

	//Index buffer subresource data
	D3D11_SUBRESOURCE_DATA indexSD = {};
	indexSD.pSysMem = indices;

	//Creating index buffer descriptor
	D3D11_BUFFER_DESC indexBD = {};
	indexBD.ByteWidth = sizeof(indices);
	indexBD.Usage = D3D11_USAGE_DEFAULT;
	indexBD.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBD.CPUAccessFlags = 0u;
	indexBD.MiscFlags = 0u;
	indexBD.StructureByteStride = sizeof(int);

	//Creating and setting index buffer
	Microsoft::WRL::ComPtr<ID3D11Buffer> pIndexBuffer;
	GFX_THROW_INFO(pDevice->CreateBuffer(&indexBD, &indexSD, &pIndexBuffer));
	pContext->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);

	//Defining the constant data for transformation constant buffer
	struct ConstantBuffer
	{
		DirectX::XMMATRIX transformation;
	};
	const ConstantBuffer constBuf =
	{
		DirectX::XMMatrixTranspose(
			DirectX::XMMatrixRotationY(rotY) *
			DirectX::XMMatrixRotationX(rotX) * 
			DirectX::XMMatrixScaling(3.0f / 4.0f,1.0f,1.0f) *
			DirectX::XMMatrixTranslation(posX, posY, posZ) *
			DirectX::XMMatrixPerspectiveLH(1.0f,1.0f,0.5f,10.0f) //FOV
		)
	};

	//Constant data for color constant buffer
	struct ConstantBuffer2
	{
		struct color
		{
			float r;
			float g;
			float b;
			float a;
		} colors[5];
	};

	const ConstantBuffer2 constBuf2 =
	{
		{
			{1.0f,0.0f,0.0f,1.0f},
			{0.0f,0.0f,1.0f,1.0f},
			{1.0f,0.0f,1.0f,1.0f},
			{1.0f,0.0f,1.0f,1.0f},
			{1.0f,0.0f,1.0f,1.0f}
		}
	};

		//Transformation Constant Buffer
	//Creating constant buffer subresource data
	D3D11_SUBRESOURCE_DATA constantSD = {};
	constantSD.pSysMem = &constBuf;

	//Creating constant buffer descriptor
	D3D11_BUFFER_DESC constantBD = {};
	constantBD.ByteWidth = sizeof(constBuf);
	constantBD.Usage = D3D11_USAGE_DYNAMIC;
	constantBD.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBD.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBD.MiscFlags = 0u;
	constantBD.StructureByteStride = 0u;

	//Creating and setting the constant buffer for transformation matrix
	Microsoft::WRL::ComPtr<ID3D11Buffer> pConstantBuffer;
	GFX_THROW_INFO(pDevice->CreateBuffer(&constantBD, &constantSD, &pConstantBuffer));
	pContext->VSSetConstantBuffers(0u, 1u, pConstantBuffer.GetAddressOf());

		//Color Constant Buffer
	//Creating constant buffer subresource data
	D3D11_SUBRESOURCE_DATA constantSD2 = {};
	constantSD2.pSysMem = &constBuf2;

	//Creating constant buffer descriptor
	D3D11_BUFFER_DESC constantBD2 = {};
	constantBD2.ByteWidth = sizeof(constBuf2);
	constantBD2.Usage = D3D11_USAGE_DYNAMIC;
	constantBD2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBD2.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBD2.MiscFlags = 0u;
	constantBD2.StructureByteStride = 0u;

	//Creating and setting the constant buffer for transformation matrix
	Microsoft::WRL::ComPtr<ID3D11Buffer> pConstantBuffer2;
	GFX_THROW_INFO(pDevice->CreateBuffer(&constantBD2, &constantSD2, &pConstantBuffer2));
	pContext->PSSetConstantBuffers(0u, 1u, pConstantBuffer2.GetAddressOf());
	
	//Creating the blob for loading the bytecodes for input layout, vertex shader and pixel shader
	Microsoft::WRL::ComPtr<ID3DBlob> pBlobContents;

	//Loading the pixel shader bytecode
	D3DReadFileToBlob(L"PixelShader.cso", &pBlobContents);

	//Creating and Setting the pixel shader
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader;
	GFX_THROW_INFO(pDevice->CreatePixelShader(pBlobContents->GetBufferPointer(), pBlobContents->GetBufferSize(), nullptr, &pPixelShader));
	pContext->PSSetShader(pPixelShader.Get(), nullptr, 0u);
	
	//Loading the vertex shader bytecode
	D3DReadFileToBlob(L"VertexShader.cso", &pBlobContents);

	//Creating and setting the vertex shader
	Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader;
	GFX_THROW_INFO(pDevice->CreateVertexShader(pBlobContents->GetBufferPointer(), pBlobContents->GetBufferSize(), nullptr, &pVertexShader));
	pContext->VSSetShader(pVertexShader.Get(), nullptr, 0u);

	//Creating the descriptor for input layout
	const D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{"Position",0u,DXGI_FORMAT_R32G32B32_FLOAT,0u,0u,D3D11_INPUT_PER_VERTEX_DATA,0u}
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

	//Drawing
	GFX_THROW_ONLY_INFO( pContext->DrawIndexed((UINT)std::size(indices),0u,0u) );
	//GFX_THROW_ONLY_INFO(pContext->Draw((UINT)std::size(vertices),0u));
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
