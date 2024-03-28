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
	window.Gfx().DrawTriangle(timer.Peek(), window.mouse.GetPosX() / 400.0f - 1, window.mouse.GetPosY() / -300.0f +1 );
	window.Gfx().EndFrame();
}
