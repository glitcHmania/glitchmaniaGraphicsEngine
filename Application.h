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
	bool dragFlag = false;
	float inPosX = 0.0f, inPosY = 0.0f;
	float inRotX = 0.0f, inRotY = 0.0f;
	float rotMultiplier = 3.0f;
	float movAmount = 0.1f;
	float rotX = 0.0f;
	float rotY = 0.0f;
	float posX = 0.0f;
	float posY = 0.0f;
	float posZ = 1.5f;
};

