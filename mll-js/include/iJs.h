#ifndef __ILUA_H__
#define __ILUA_H__
#include <mll/mll.h>
#include <duktape.h>
#include <duk_config.h>

class iJs : public mlInterpreter
{
	private:
		duk_context* ctx = NULL;
	//	void add_lib(std::string name, lua_CFunction func);
	public:
		void* get_id() {return (void*)ctx;}
		iJs();
		bool eval(std::string str);
		bool exec(std::string filename);
		bool set(std::string name, mlVariable val);
		void push_to_stack(mlVariable val);
		mlVariable  get_from_stack(int idx, int type = -1);
		mlVariable  get(std::string name);
		/*
		lua_Debug	get_debug_info();
		std::vector<mlVariable> call(std::string functionname,std::vector<mlVariable>& args);
		std::vector<mlVariable> call(mlVariable func,std::vector<mlVariable>& args);*/
};

extern "C"
{
	iJs* mllib_generate();
}
#endif
