#pragma once

#include <stdexcept>

class CException : public std::exception
{
public:
	CException(const char* format, ...);
	~CException();

	const char* what() const throw() override;
private:
	const char* _exception = NULL;
};
