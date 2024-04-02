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
	// Move
	if (window.kbd.KeyIsPressed('D'))
	{
		posX += movAmount;
	}
	else if (window.kbd.KeyIsPressed('A'))
	{
		posX -= movAmount;
	}
	if (window.kbd.KeyIsPressed('W'))
	{
		posY += movAmount;
	}
	else if (window.kbd.KeyIsPressed('S'))
	{
		posY -= movAmount;
	}

	// Rotate
	auto msEvent = window.mouse.Read().GetType();
	if (!dragFlag && window.mouse.LeftIsPressed())
	{
		inPosX = window.mouse.GetPosX();
		inPosY = window.mouse.GetPosY();
		inRotX = rotX;
		inRotY = rotY;
		dragFlag = true;
	}
	else if (dragFlag && msEvent == Mouse::Event::Type::Move)
	{
		rotX = inRotX + (inPosY - window.mouse.GetPosY()) * rotMultiplier / 300.0f;
		rotY = inRotY + (inPosX - window.mouse.GetPosX()) * rotMultiplier / 400.0f;
		dragFlag = false;
	}

	// Zoom-In & Zoom-Out
	if (msEvent == Mouse::Event::Type::WheelDown)
	{
		posZ += movAmount;
	}
	else if (msEvent == Mouse::Event::Type::WheelUp)
	{
		posZ -= movAmount;
	}

	const float c = std::sin(timer.Peek()) /2.0f ;
	window.Gfx().ClearBuffer(0.2f, 0.6f, 1.0f);
	window.Gfx().DrawTriangle(posX,posY,posZ,rotX,rotY);
	window.Gfx().EndFrame();
}
