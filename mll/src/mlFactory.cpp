#include <iostream>
#include <algorithm>
extern "C" {
	#include <sys/stat.h>
}
#include <mll/mlException.h>
#include <mll/mlFactory.h>

std::vector <std::string> mlFactory::path;
std::vector <mlLib*> mlFactory::libs;
bool mlFactory::exceptions = false;

std::string mlFactory::search_file(std::string path)
{
	struct stat buf;
	if (stat(path.c_str(), &buf) != 0)
		return path;
	for (int i=0;i<mlFactory::path.size();i++)
		if (stat((mlFactory::path[i] + path).c_str(), &buf) != 0)
			return (mlFactory::path[i] + path);
	return path;
}

struct mlLibSearcher
{
		mlLibSearcher(mlLib* ptr) : search_ptr(ptr) {}
		bool operator()(mlLib* l) {return (*l == *search_ptr);}
	private:
		mlLib* search_ptr;
};
bool mlFactory::is_loaded(mlLib* lib)
{
	return (std::find_if(libs.begin(), libs.end(), mlLibSearcher(lib)) != libs.end());
}

mlLib* mlFactory::load(std::string name)
{
	name = search_file(name);	
	mlLib *l = new mlLib(name);
	if (!l->is_good())
	{
		delete l;
		if (exceptions) throw mlException(l->error());
		else return NULL;
	}
	
	if (is_loaded(l))
	{
		delete l;
		return NULL;
	}

	if (!l->load() || !l->iregister())
	{
		delete l;
		if (exceptions) throw mlException(l->error());
		else return NULL;
	}
	
	libs.push_back(l);
	return l;
}

void mlFactory::dump(std::ostream& stream)
{
	stream << "Connected libraryes: " << libs.size() << "\n\t"
	<< "Info: " << std::endl;
	for (int i=0;i<libs.size();i++)
		stream << *libs[i] << std::endl;
}

mlInterpreter* mlFactory::create_interpreter(std::string name)
{
	for (int i=0;i<libs.size();i++)
		if (libs[i]->get_name() == name)
			return libs[i]->create_interpreter();
}

mlInterpreter* mlFactory::search(void* identifier)
{
	mlInterpreter* interp;
	for (int i=0;i<libs.size();i++)
		if ((interp = libs[i]->search(identifier)) != NULL)
			return interp;
	return NULL;
}
