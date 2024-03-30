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
	float rotY = 0.0f;
	float rotX = 0.0f;
	float posZ = 1.5f;
};

