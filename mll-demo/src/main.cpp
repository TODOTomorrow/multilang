#include <iostream>
#include <tuple>
#include <stdexcept>
#include <functional>
#include <mll/mll.h>
#include <mll/mlSupport.h>

using namespace std;

class TestA
{
	public:
		TestA(std::string v) {std::cout << v << std::endl;}
		int print() {cout << "Is A!";}
};

void test_function_without_conversion(mlVariable a, mlVariable x)
{
	cout << x << " , " << a << endl;
}

void test_function(std::string x, int a)
{
	cout << x << " , " << a << endl;
}

void test_function_with_raw_arguments(std::vector<mlVariable> args)
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
		intrptr->set("test_function",test_function);
		intrptr->set("test_function_with_raw_arguments",test_function_with_raw_arguments);
		intrptr->set("test_function_without_conversion",test_function_without_conversion);
		
		mlVariable test_number_variable = 1;
		mlVariable test_string_variable = "Test string";
		mlVariable test_function_variable = test_function;
		mlVariable test_object_variable;
		test_object_variable["number_field"] = 1;
		test_object_variable["string_field"] = "String field";
		intrptr->set("test_number_variable", test_number_variable);
		intrptr->set("test_string_variable", test_string_variable);
		intrptr->set("test_function_variable", test_function_variable);
		intrptr->set("test_object_variable", test_object_variable);
		
		std::cout << "Test calling mlVariable callback from C++ function" << std::endl;
		test_function("Some args",2);
		
		intrptr->class_register<TestA,std::string>("A");
		intrptr->method_register("print",&TestA::print);
		intrptr->exec("test.js");
	}
	catch (mlException& e)
	{
		cout << e.what() << endl;
	}
}
