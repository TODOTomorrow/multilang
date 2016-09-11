extern "C"
{
	#include <dlfcn.h>
}
#include <iostream>
#include <mll/mlLib.h>

mlLib::mlLib(std::string path)
{
	lib_ptr = dlopen(path.c_str() , RTLD_LAZY);
	if (lib_ptr == NULL)
		this->error_message = dlerror();
	good = true;
}

bool mlLib::load()
{
	generate_func = (decltype(generate_func))dlsym(lib_ptr, "mllib_generate");
	get_info_func = (decltype(get_info_func))dlsym(lib_ptr, "mllib_get_info");
	char* errmsg;
	
	if ( (errmsg = dlerror()) != NULL)
	{
		this->error_message = errmsg;
		this->error_message += "Cannot find required symbols";
		good = false;
		return false;
	}
	return true;
}

bool mlLib::iregister()
{
	if (generate_func == NULL || get_info_func == NULL)
	{
		this->error_message += "Cannot find required symbols to get info";
		good = false;
		return false;
	}
	error_message = "A";
	this->info = get_info_func();
	this->capabilities = this->info->get_capabilities();
	return true;
}
mlInterpreter* mlLib::create_interpreter()
{
	mlInterpreter* interp;
	if (generate_func == NULL) return NULL;
	interp = generate_func();
	runtimes.push_back(interp);
	interp->set_owner(this);
	return interp;
}
mlInterpreter* mlLib::search(void* identiefer)
{
	for (int i=0;i<runtimes.size();i++)
		if (*(runtimes[i]) == identiefer)
			return runtimes[i];
	return NULL;
}
