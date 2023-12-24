#include "NonMacroWin.h"
#include <string>
#include "WindowsMessageMap.h"
#include "Window.h"
#include "GGException.h"
#include "Application.h"
#include <sstream>

int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd)
{
	try
	{
		return Application{}.Run();
	}
	catch (const GGException& exc)
	{
		MessageBox(nullptr, exc.what(), exc.GetType(), MB_OKCANCEL | MB_ICONEXCLAMATION);
	}
	catch (const std::exception exc)
	{
		MessageBox(nullptr, exc.what(), "Standart Exception", MB_OKCANCEL | MB_ICONEXCLAMATION);
	}
	catch (...)
	{
		MessageBox(nullptr, "No details", "Unknown Exception", MB_OKCANCEL | MB_ICONEXCLAMATION);
	}
	return -1;
}