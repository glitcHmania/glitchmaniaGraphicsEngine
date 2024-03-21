#include "Application.h"
#include <ostream>
#include "GGTimer.h"

Application::Application()
	:
	window(800, 600, "Glitchmania Graphics Engine")
{
}

int Application::Run()
{
	while(true)
	{
		//If the optional exitCode has value it return true. If it is true return the WM_QUIT, otherwise call ComposeFrame() and continue
		if (const auto exitCode = Window::ProcessMessages())
		{
			return *exitCode;
		}
		ComposeFrame();
	}
}

void Application::ComposeFrame()
{
	const float c = std::sin(timer.Peek()) /2.0f ;
	window.Gfx().ClearBuffer(0.0f, 0.0f, 0.0f);
	window.Gfx().DrawTriangle(c);
	window.Gfx().EndFrame();
}
