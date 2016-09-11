#ifndef __ML_FACTORY_H__
#define __ML_FACTORY_H__
#include <vector>
#include <ostream>
#include <mll/mlInterpreter.h>
#include <mll/mlLib.h>

class mlFactory
{
	private:
		static std::vector <std::string> path;
		static std::vector <mlLib*> libs;
		
		static std::string search_file(std::string path);
		static bool is_loaded(mlLib* lib);
		static void iregister(mlInterpreter* interpreter);
	public:
		static bool exceptions;
		static mlLib* load(std::string path);
		static void dump(std::ostream& stream);
		static mlInterpreter* create_interpreter(std::string name);
		static mlInterpreter* search(void* identifier);
};

#endif
