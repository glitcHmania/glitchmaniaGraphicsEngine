#include "Mouse.h"
#include "Window.h"

Mouse::~Mouse()
{
}

std::pair<int, int> Mouse::GetPos() const noexcept
{
	return std::pair<int, int>(x,y);
}

int Mouse::GetPosX() const noexcept
{
	return x;
}

int Mouse::GetPosY() const noexcept
{
	return y;
}

bool Mouse::LeftIsPressed() const noexcept
{
	return leftIsPressed;
}

bool Mouse::RightIsPressed() const noexcept
{
	return rightIsPressed;
}

bool Mouse::MiddleIsPressed() const noexcept
{
	return middleIsPressed;
}

Mouse::Event Mouse::Read() noexcept
{
	if (buffer.size() > 0u)
	{
		Mouse::Event e = buffer.front();
		buffer.pop();
		return e;
	}
	else
	{
		return Mouse::Event();
	}
}

bool Mouse::IsEmpty() const noexcept
{
	return buffer.empty();
}

void Mouse::Clear() noexcept
{
	buffer = std::queue<Event>();
}

void Mouse::OnMouseMove(int x, int y) noexcept
{
	this->x = x;
	this->y = y;
	buffer.push(Mouse::Event(Mouse::Event::Type::Move, *this));
	TrimBuffer();
}

void Mouse::OnLeftPressed(int x, int y) noexcept
{
	leftIsPressed = true;
	buffer.push(Mouse::Event(Mouse::Event::Type::LPress, *this));
	TrimBuffer();
}

void Mouse::OnLeftReleased(int x, int y) noexcept
{
	leftIsPressed = false;
	buffer.push(Mouse::Event(Mouse::Event::Type::LRelease, *this));
	TrimBuffer();
}

void Mouse::OnRightPressed(int x, int y) noexcept
{
	rightIsPressed = true;
	buffer.push(Mouse::Event(Mouse::Event::Type::RPress, *this));
	TrimBuffer();
}

void Mouse::OnRightReleased(int x, int y) noexcept
{
	rightIsPressed = false;
	buffer.push(Mouse::Event(Mouse::Event::Type::RRelease, *this));
	TrimBuffer();
}

void Mouse::OnMiddlePressed(int x, int y) noexcept
{
	middleIsPressed = true;
	buffer.push(Mouse::Event(Mouse::Event::Type::MPress, *this));
	TrimBuffer();
}

void Mouse::OnMiddleReleased(int x, int y) noexcept
{
	middleIsPressed = false;
	buffer.push(Mouse::Event(Mouse::Event::Type::MRelease, *this));
	TrimBuffer();
}

void Mouse::OnWheelDelta(int x, int y, int delta) noexcept
{
	accumulatedWheelDelta += delta;
	while (accumulatedWheelDelta >= WHEEL_DELTA)
	{
		accumulatedWheelDelta -= WHEEL_DELTA;
		OnWheelUp(x, y);
	}
	while (accumulatedWheelDelta <= -WHEEL_DELTA)
	{
		accumulatedWheelDelta += WHEEL_DELTA;
		OnWheelDown(x, y);
	}
}

void Mouse::OnWheelUp(int x, int y) noexcept
{
	buffer.push(Mouse::Event(Mouse::Event::Type::WheelUp, *this));
	TrimBuffer();
}

void Mouse::OnWheelDown(int x, int y) noexcept
{
	buffer.push(Mouse::Event(Mouse::Event::Type::WheelDown, *this));
	TrimBuffer();
}

bool Mouse::IsInside() const noexcept
{
	return isInside;
}

void Mouse::OnEnter() noexcept
{
	isInside = true;
	buffer.push(Mouse::Event(Mouse::Event::Type::Enter, *this));
	TrimBuffer();
}

void Mouse::OnLeave() noexcept
{
	isInside = false;
	buffer.push(Mouse::Event(Mouse::Event::Type::Leave, *this));
	TrimBuffer();
}

void Mouse::TrimBuffer() noexcept
{
	while (buffer.size() > bufferSize)
	{
		buffer.pop();
	}
}
