#include "GGException.h"
#include <sstream>

GGException::GGException(int line, const char* file) noexcept
	:
	line(line),
	file(file)
{
}

const char* GGException::what() const noexcept
{
	std::ostringstream stream;
	stream << GetType() << std::endl << GetOrigin();
	whatBuffer = stream.str();
	return whatBuffer.c_str();
}

const char* GGException::GetType() const noexcept
{
	return "GMG Exception";
}

int GGException::GetLine() const noexcept
{
	return line;
}

const std::string& GGException::GetFile() const noexcept
{
	return file;
}

std::string GGException::GetOrigin() const noexcept
{
	std::ostringstream stream;
	stream << "[File] " << file << std::endl << "[Line] " << line;
	return stream.str();
}

