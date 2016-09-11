#ifndef __MLLIB_H__
#define __MLLIB_H__
#include <mll/mlInterpreterInfo.h>
#include <mll/mlInterpreter.h>
#include <mll/mlSupport.h>
class mlLib
{
	private:
		void* lib_ptr;
		mlInterpreterInfo* info;
		mlInterpreter* (*generate_func)() = NULL;
		mlInterpreterInfo* (*get_info_func)() = NULL;
		bool good;
		std::string error_message;
		std::vector<mlInterpreter*> runtimes;
		mlInterpreterInfo::capabilities_t capabilities;
	public:
		mlInterpreterInfo::capabilities_t get_capabilities() {return capabilities;}
		mlLib(std::string path);
		char* error() {return (char*)error_message.c_str();}
		bool load();
		bool is_good() {return (lib_ptr != NULL && good);}
		bool operator ==(mlLib& l) {return (l.lib_ptr == this->lib_ptr);}
		bool iregister();
		std::string get_name() {return ((info!=NULL)?info->get_name():"");}
		
		mlInterpreter* create_interpreter();
		mlInterpreter* search(void* identiefer);
		friend std::ostream& operator<< (std::ostream& stream, const mlLib& lib)
		{
			stream << "Interpreter library information:\n\tloaded:" <<
			((lib.lib_ptr != NULL && lib.generate_func != NULL && lib.get_info_func != NULL)?"yes":"no") <<
			"\n\t" << "Info:\n" << *lib.info  << std::endl;
			return stream;
		}
};

#endif
