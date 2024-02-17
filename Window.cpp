#include "Window.h"
#include <sstream>
#include "resource.h"
Window::WindowClass Window::WindowClass::wndClass;

Window::WindowClass::WindowClass() noexcept
	:
	hInst(GetModuleHandle(nullptr))
{
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = HandleMsgSetup;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetInstance();
	wc.hIcon = static_cast<HICON>(LoadImage(GetInstance(), MAKEINTRESOURCE(IDI_ICON2), IMAGE_ICON, 32, 32, 0));
	wc.hCursor = nullptr;
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = GetName();
	wc.hIconSm = static_cast<HICON>(LoadImage(GetInstance(), MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 16, 16, 0));
	RegisterClassEx(&wc);
}

Window::WindowClass::~WindowClass()
{
	UnregisterClass(wndClassName, GetInstance());
}

const char* Window::WindowClass::GetName() noexcept
{
	return wndClassName;
}

HINSTANCE Window::WindowClass::GetInstance() noexcept
{
	return wndClass.hInst;
}

Window::Window(int width, int height, const char* name)
	:
	width(width),
	height(height)
{
	//Calculate window size based on desired client region size
	RECT wr;
	wr.left = 100;
	wr.right = width + wr.left;
	wr.top = 100;
	wr.bottom = height + wr.top;
	if (AdjustWindowRect(&wr, WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU, FALSE) == 0)
	{
		throw GGE_LAST_EXCEPT();
	}
	//Create window & get hWnd
	hWnd = CreateWindow(
		WindowClass::GetName(),
		name,
		WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT,
		wr.right - wr.left, wr.bottom - wr.top,
		nullptr, nullptr, WindowClass::GetInstance(), this);
	if (hWnd == nullptr)
	{
		throw GGE_LAST_EXCEPT();
	}
	// Show window
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	//Create gfx object
	pGfx = std::make_unique<Graphics>(hWnd);
}

Window::~Window()
{
	DestroyWindow(hWnd);
}

void Window::SetTitle(const std::string& title)
{
	if (SetWindowText(hWnd, title.c_str()) == 0)
	{
		throw GGE_LAST_EXCEPT();
	}
}

std::optional<int> Window::ProcessMessages()
{
	MSG msg;
	//Process all messages in queue, but do not block for new messages
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		//Check for quit because peekmessage does not signal this via return val
		if (msg.message == WM_QUIT)
		{
			return msg.wParam;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		

	}
	//Return empty optional when not quitting app
	return {};
}

Graphics& Window::Gfx()
{
	if (!pGfx)
	{
		throw GGE_NOGFX_EXCEPT();
	}
	return *pGfx;
}

LRESULT CALLBACK Window::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	//Use create parameter passed in from CreateWindow() to store window class pointer at WinAPI side
	if (msg == WM_NCCREATE)
	{
		//Extract ptr to window class from creation data
		const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
		Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);
		//Set WinAPI-managed user data to store ptr to window class
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
		//Set message proc to normal (non-setup) handler now that setup is finished
		SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgThunk));
		//Forward message to window class handler
		return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
	}
	//If we get a message before the WM_NCCREATE message, handle with default handler
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK Window::HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	//Retrieve ptr to window class
	Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	//Forward message to window class handler
	return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
}

LRESULT Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	switch (msg)
	{
	//Window messages
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;	
	case WM_KILLFOCUS:
		kbd.ClearState();
		break;
	//Keyboard messages
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		//Checking bit 30 of lParam to check if the key is being held down
		if (!(lParam & 0x40000000) || kbd.IsKeysAutoRepeat())
		{
			kbd.OnKeyPressed(static_cast<unsigned char>(wParam));
		}
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		kbd.OnKeyReleased(static_cast<unsigned char>(wParam));
		break;
	case WM_CHAR:
		kbd.OnChar(static_cast<char>(wParam));
		break;
	//Mouse messages
	case WM_MOUSEMOVE:
		const POINTS pt = MAKEPOINTS(lParam);
		if(pt.x<width && pt.x>0 && pt.y<height && pt.y>0)
		{
			mouse.OnMouseMove(pt.x, pt.y);
			if (!mouse.IsInside())
			{
				SetCapture(hWnd);
				mouse.OnEnter();
			}
		}
		else
		{
			if (mouse.LeftIsPressed() || mouse.RightIsPressed() || mouse.MiddleIsPressed())
			{
				mouse.OnMouseMove(pt.x, pt.y);
			}
			else
			{
				ReleaseCapture();
				mouse.OnLeave();
			}
		}

		mouse.OnMouseMove(pt.x, pt.y);
		break;
	case WM_LBUTTONDOWN:
		const POINTS ptL = MAKEPOINTS(lParam);
		mouse.OnLeftPressed(ptL.x, ptL.y);
		break;
	case WM_RBUTTONDOWN:
		const POINTS ptR = MAKEPOINTS(lParam);
		mouse.OnRightPressed(ptR.x, ptR.y);
		break;
	case WM_MBUTTONDOWN:
		const POINTS ptM = MAKEPOINTS(lParam);
		mouse.OnMiddlePressed(ptM.x, ptM.y);
		break;
	case WM_LBUTTONUP:
		const POINTS ptLU = MAKEPOINTS(lParam);
		mouse.OnLeftReleased(ptLU.x, ptLU.y);
		break;
	case WM_RBUTTONUP:
		const POINTS ptRU = MAKEPOINTS(lParam);
		mouse.OnRightReleased(ptRU.x, ptRU.y);
		break;
	case WM_MBUTTONUP:
		const POINTS ptMU = MAKEPOINTS(lParam);
		mouse.OnMiddleReleased(ptMU.x, ptMU.y);
		break;
	case WM_MOUSEWHEEL:
		const POINTS ptW = MAKEPOINTS(lParam);
		const int delta = GET_WHEEL_DELTA_WPARAM(wParam);
		mouse.OnWheelDelta(ptW.x, ptW.y, delta);
		break;

	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

// Window Exception Stuff
std::string Window::Exception::TranslateErrorCode(HRESULT hr) noexcept
{
	char* pMsgBuf = nullptr;
	// windows will allocate memory for err string and make our pointer point to it
	const DWORD nMsgLen = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPSTR>(&pMsgBuf), 0, nullptr
	);
	// 0 string length returned indicates a failure
	if (nMsgLen == 0)
	{
		return "Unidentified error code";
	}
	// copy error string from windows-allocated buffer to std::string
	std::string errorString = pMsgBuf;
	// free windows buffer
	LocalFree(pMsgBuf);
	return errorString;
}

Window::HrException::HrException(int line, const char* file, HRESULT hr) noexcept
	:
	Exception(line, file),
	hr(hr)
{}

const char* Window::HrException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
		<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
		<< "[Description] " << GetErrorDescription() << std::endl
		<< GetOrigin();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Window::HrException::GetType() const noexcept
{
	return "GGE Window Exception";
}

HRESULT Window::HrException::GetErrorCode() const noexcept
{
	return hr;
}

std::string Window::HrException::GetErrorDescription() const noexcept
{
	return Exception::TranslateErrorCode(hr);
}


const char* Window::NoGfxException::GetType() const noexcept
{
	return "GGE Window Exception - No Graphics";
}