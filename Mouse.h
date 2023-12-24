#pragma once
#include <queue>

class Mouse
{
	friend class Window;
public:
	class Event
	{
	public:
		enum class Type
		{
			LPress,
			LRelease,
			RPress,
			RRelease,
			WheelUp,
			WheelDown,
			MPress,
			MRelease,
			Move,
			Enter,
			Leave,
			Invalid
		};
		Event() noexcept
			:
			type(Type::Invalid),
			leftIsPressed(false),
			rightIsPressed(false),
			middleIsPressed(false),
			x(0),
			y(0)
		{}
		Event(Type type, const Mouse& parent) noexcept
			:
			type(type),
			leftIsPressed(parent.leftIsPressed),
			rightIsPressed(parent.rightIsPressed),
			middleIsPressed(parent.middleIsPressed),
			x(parent.x),
			y(parent.y)
		{}
		Event(Type type, bool leftIsPressed, bool rightIsPressed, bool middleIsPressed, int x, int y) noexcept
			:
			type(type),
			leftIsPressed(leftIsPressed),
			rightIsPressed(rightIsPressed),
			middleIsPressed(middleIsPressed),
			x(x),
			y(y)
		{}
		Type GetType() const noexcept
		{
			return type;
		}
		bool IsValid() const noexcept
		{
			return type != Type::Invalid;
		}
		bool LeftIsPressed() const noexcept
		{
			return leftIsPressed;
		}
		bool RightIsPressed() const noexcept
		{
			return rightIsPressed;
		}
		bool MiddleIsPressed() const noexcept
		{
			return middleIsPressed;
		}
		int GetPosX() const noexcept
		{
			return x;
		}
		int GetPosY() const noexcept
		{
			return y;
		}
		std::pair<int, int> GetPos() const noexcept
		{
			return { x,y };
		}
	private:
		Type type;
		bool leftIsPressed;
		bool rightIsPressed;
		bool middleIsPressed;
		int x;
		int y;
	};
	public:
		Mouse() = default;
		Mouse(const Mouse&) = delete;
		Mouse& operator=(const Mouse&) = delete;
		~Mouse();
		std::pair<int, int> GetPos() const noexcept;
		int GetPosX() const noexcept;
		int GetPosY() const noexcept;
		bool LeftIsPressed() const noexcept;
		bool RightIsPressed() const noexcept;
		bool MiddleIsPressed() const noexcept;
		Mouse::Event Read() noexcept;
		bool IsEmpty() const noexcept;
		void Clear() noexcept;
	private:
		void OnMouseMove(int x, int y) noexcept;
		void OnLeftPressed(int x, int y) noexcept;
		void OnLeftReleased(int x, int y) noexcept;
		void OnRightPressed(int x, int y) noexcept;
		void OnRightReleased(int x, int y) noexcept;
		void OnMiddlePressed(int x, int y) noexcept;
		void OnMiddleReleased(int x, int y) noexcept;
		void OnWheelDelta(int x, int y, int delta) noexcept;
		void OnWheelUp(int x, int y) noexcept;
		void OnWheelDown(int x, int y) noexcept;
		bool IsInside() const noexcept;
		void OnEnter() noexcept;
		void OnLeave() noexcept;
		void TrimBuffer() noexcept;
	private:
		static constexpr unsigned int bufferSize = 16u;
		int x;
		int y;
		bool leftIsPressed = false;
		bool rightIsPressed = false;
		bool middleIsPressed = false;
		bool isInside = false;
		int accumulatedWheelDelta = 0;
		std::queue<Event> buffer;
};

