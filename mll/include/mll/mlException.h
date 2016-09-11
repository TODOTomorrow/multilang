#ifndef __MLEXCEPTION_H__
#define __MLEXCEPTION_H__
#include <exception>
#include <string>

class mlException : public std::exception
{
	protected:
		char* message;
	public:
		mlException()
		{
		}
		mlException(const char* wu) { message = (char*)wu; }
		mlException(char* wu)       { message = wu; }
		mlException(std::string s)  { message = (char*)s.c_str();}
		virtual const char* what()
		{
			return (std::string("Multi language exception.\nException: ") + message).c_str();
		}
};

#endif
