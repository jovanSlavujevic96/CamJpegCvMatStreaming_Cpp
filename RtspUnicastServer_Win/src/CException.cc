#include "CException.h"

#include <cstdarg>
#include <cstdlib>
#include <cstdio>

CException::CException(const char* format, ...)
{
	if (NULL == format)
	{
		return;
	}

	/* create arguments lists and extract them via format */
	std::va_list args, args2;
	va_start(args, format);
	va_copy(args2, args);

	/* get passed format exception's size then allocate exception string if anything exists */
	const int size = std::vsnprintf(NULL, 0, format, args) + 1;
	if (size > 1)
	{
		_exception = (const char*)malloc(size);
		if (_exception)
		{
			(void)std::vsnprintf((char*)_exception, size, format, args2);
		}
	}

	/* free arguments lists */
	va_end(args);
	va_end(args2);
}

CException::~CException()
{
	free((void*)_exception);
}

const char* CException::what() const throw()
{
	return _exception;
}
