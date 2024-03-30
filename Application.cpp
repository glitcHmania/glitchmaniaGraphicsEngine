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
	float rotAmount = 0.05f;
	float movAmount = 0.1f;

	if (window.kbd.KeyIsPressed(0x41))
	{
		rotY += rotAmount;
	}
	else if (window.kbd.KeyIsPressed(0x44))
	{
		rotY -= rotAmount;
	}
	if (window.kbd.KeyIsPressed(0x57))
	{
		rotX += rotAmount;
	}
	else if (window.kbd.KeyIsPressed(0x53))
	{
		rotX -= rotAmount;
	}

	auto msEvent = window.mouse.Read().GetType();
	if (msEvent == Mouse::Event::Type::WheelDown)
	{
		posZ -= movAmount;
	}
	else if (msEvent == Mouse::Event::Type::WheelUp)
	{
		posZ += movAmount;
	}

	const float c = std::sin(timer.Peek()) /2.0f ;
	window.Gfx().ClearBuffer(0.0f, 0.0f, 0.0f);
	window.Gfx().DrawTriangle(rotX, rotY, window.mouse.GetPosX() / 400.0f - 1, window.mouse.GetPosY() / -300.0f +1, posZ);
	window.Gfx().EndFrame();
}
