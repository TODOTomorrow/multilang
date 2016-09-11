#include <iostream>
#include <tuple>
#include <stdexcept>
#include <functional>
#include <mll/mll.h>
#include <mll/mlSupport.h>

using namespace std;
mlVariable func(std::string fn, std::vector<mlVariable> args, mlInterpreter* owner)
{
	std::cout << "Hello from C! Function called: " << fn << " Arguments count: " << args.size() << std::endl;
	for (int i=0;i<args.size();i++)
	{
		cout << args[i] << endl;
	}
	return mlVariable("Okay its answer from C");
}

class TestA
{
	public:
		void print() {cout << "Is A!";}
};

void test_function(mlVariable a, mlVariable x)
{
	cout << x << " , " << a << endl;
}

void test_function2(std::string x, int a)
{
	cout << x << " , " << a << endl;
}

int main(int argc,char** argv)
{
	if (argc == 1) return 0;
	try
	{
		mlFactory::exceptions = true;
		mlLib* l = mlFactory::load(string(argv[1]));
		mlInterpreter* intptr = mlFactory::create_interpreter("Javascript");
		init_introspector(intptr);
		mlVariable x;
		x["a"] = 1;
		x["func"] = test_function;
		x["b"] = -1;
		x["func"] = func;
		//x = func;
		
		intptr->set("x",x);
		//intptr->class_register<TestA>("TestA");
		mlVariable f = test_function;
		intptr->set("func",f);
		f("Hello");
		intptr->exec("test.js");
	}
	catch (mlException& e)
	{
		cout << e.what() << endl;
	}
}
