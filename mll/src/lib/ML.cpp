#include <map>
#include <mll/mlInterpreter.h> 
#include <mll/mlSupport.h>
#include <ffi.h>


mlVariable* register_func(std::vector<mlVariable> args)
{
	if (args.size() < 2)
		return mlVariable();
	//function_table[args[0]] = new sup_c_function(
}


void init_introspector(mlInterpreter* in)
{
	mlVariable v;
	
}
