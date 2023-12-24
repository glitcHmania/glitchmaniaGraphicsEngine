#include "Application.h"
#include <ostream>

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
	const float t = timer.Peek();
	std::ostringstream oss;
	oss << "Time elapsed: " << std::setprecision(1) << std::fixed << t << "s";
	window.SetTitle(oss.str());
}
