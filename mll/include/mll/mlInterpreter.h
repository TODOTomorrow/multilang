#ifndef __MLINTERPRETER_H__
#define __MLINTERPRETER_H__
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <mll/mlVariable.h>
#include <mll/mlInterpreterInfo.h>
#include <map>
#include <mll/mlCallback.h>

class mlLib;
class mlVariable;
class mlInterpreter;

class mlCallbackContext
{
	public:
		mlInterpreter* runtime;
		std::string function_name;
};

class mlInterpreter
{
	protected:
		std::vector<std::string> path;
		std::string search_file(std::string filename);
		std::ostream *iout, *ierr;
		std::istream* iin;
		mlLib* owner;
	public:
		void set_owner(mlLib* owner) {this->owner = owner;}
		mlInterpreter();
		template <typename T>
			bool set(std::string name, T val, mlVariable::type_t type, mlVariable::subtype_t subtype) {mlVariable v(val); v.type = type; v.subtype = subtype; v.set_owner(this); set(name,v);};
		template <typename T>
			bool set(std::string name, T val) {mlVariable v(val); v.set_owner(this); set(name,v);};
		template <typename T>
			T  get(std::string name) {mlVariable v = get(name); v.set_owner(this); return v.get<T>();};
		
		void set_out(std::ostream* s) {iout = s;}
		void set_err(std::ostream* s) {ierr = s;}
		void set_in(std::istream* s)  {iin = s;}
		
		virtual void* get_id() {};
		virtual std::string get_name() {};
		virtual bool eval(std::string string);
		virtual bool exec(std::string filename);
		
		virtual bool set(std::string name, mlVariable val) {};
		virtual mlVariable  get(std::string name) {};
		
		bool operator ==(void* id) { return (get_id() == id); }
		
		mlVariable callback(mlVariable callbackKey, std::vector<mlVariable>& args);
		void callback_register(mlVariable key, mlVariable callback);
		
		template <class T>
		static mlVariable constructor_callback(std::vector<mlVariable> args, mlCallbackContext* context)
		{
			mlVariable obj;
			mlInterpreter* runtime = context->runtime;
			if (context == NULL || context->runtime == NULL || context->function_name == "")
				throw mlException("Cannot use classed if interpreter does not support class context saving");
			
			std::cout << "Create new " << context->function_name << "()" << std::endl;
			obj["this_ptr"] = (void*)(new T());
			return obj;
		}
};
#endif
