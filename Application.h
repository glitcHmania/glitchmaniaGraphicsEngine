#pragma once
#include "Window.h"
#include "GGTimer.h"

class Application
{
public:
	Application();
	int Run();

private:
	void ComposeFrame();

private:
	Window window;
	GGTimer timer;
};

