#pragma once
#include <chrono>

class GGTimer
{
public:
	GGTimer();
	float Mark();
	float Peek() const;

private:
	std::chrono::steady_clock::time_point last;
};

