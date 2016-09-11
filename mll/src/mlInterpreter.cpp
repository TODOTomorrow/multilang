#include <fstream>
#include <climits>
#include <streambuf>
#include <mll/mlInterpreter.h>
#include <mll/mlLib.h>
#include <mll/mlCallback.h>
extern "C" {
	#include <sys/stat.h>
}
mlCallback_table mlCallbackMgr::callback_table;
bool mlInterpreter::eval(std::string string)
{
	
}

std::string mlInterpreter::search_file(std::string filename)
{
	struct stat buf;
	if (stat(filename.c_str(), &buf) != 0)
		return filename;
	for (int i=0;i<path.size();i++)
		if (stat((path[i] + filename).c_str(), &buf) != 0)
			return (path[i] + filename);
	return filename;
}

mlInterpreter::mlInterpreter() 
{
	iout = &(std::cout); 
	iin = &(std::cin); 
	ierr = &(std::cerr);
	path.push_back("./");
}


mlVariable mlInterpreter::callback(mlVariable callbackKey, std::vector<mlVariable>& args)
{
	return mlCallbackMgr::call(callbackKey,args);
}

void mlInterpreter::callback_register(mlVariable name, mlVariable callback)
{
	mlCallbackMgr::callback_register(name,callback);
}

bool mlInterpreter::exec(std::string filename)
{
	if (this->owner->get_capabilities() & mlInterpreterInfo::file_exec_no_sup)
	{
		filename = search_file(filename);
		std::ifstream f(filename.c_str());
		this->eval(std::string((std::istreambuf_iterator<char>(f)),
                 std::istreambuf_iterator<char>()));
	}
}
