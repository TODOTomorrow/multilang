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
		TestA(std::string v) {std::cout << v << std::endl;}
		int print() {cout << "Is A!";}
};

void test_function(mlVariable a, mlVariable x)
{
	cout << x << " , " << a << endl;
}

void test_function2(std::string x, int a)
{
	cout << x << " , " << a << endl;
}

void test_raw_function(std::vector<mlVariable> args)
{
	cout << "Raw function called! Args size: " << args.size() << endl;
}

int main(int argc,char** argv)
{
	if (argc == 1) return 0;
	try
	{
		mlFactory::exceptions = true;
		mlLib* l = mlFactory::load(string(argv[1]));
		mlInterpreter* intrptr = mlFactory::create_interpreter("Javascript");
		std::cout << mlCallbackMgr::C_CALLBACK_TYPE << std::endl;
		std::cout << mlCallbackMgr::CLASS_CALLBACK_TYPE << std::endl;
		init_introspector(intrptr);
		mlVariable x;
		//x["a"] = 1;
		//x["func"] = test_function;
		//x["b"] = -1;
		//x["func"] = func;
		//x = func;
		
		//intptr->set("x",x);
		intrptr->class_register<TestA,std::string>("A");
		//intrptr->method_register("print",&TestA::print);
		//intptr->class_register<TestA>("TestA");
		//mlVariable f = test_function;
		//intptr->set("func",f);
		//f("Hello");
		//intrptr->exec("test.js");
	}
	catch (mlException& e)
	{
		cout << e.what() << endl;
	}
}
