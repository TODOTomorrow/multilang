#ifndef __ILUA_H__
#define __ILUA_H__
#include <mll/mlInterpreter.h>
#include <lua.hpp>
class iLua : public mlInterpreter
{
	private:
		lua_State* ctx;
		void add_lib(std::string name, lua_CFunction func);
	public:
		iLua();
		bool eval(std::string str);
		bool exec(std::string filename);
		bool set(std::string name, mlVariable val);
		void* get_id() {return (void*)ctx;}
		
		mlVariable  get(std::string name);
		mlVariable  get_from_stack(int idx);
		void		push_to_stack(mlVariable v);
		lua_Debug	get_debug_info();
		std::vector<mlVariable> call(std::string functionname,std::vector<mlVariable>& args);
		std::vector<mlVariable> call(mlVariable func,std::vector<mlVariable>& args);
};

extern "C"
{
	iLua* mllib_generate();
}
#endif
