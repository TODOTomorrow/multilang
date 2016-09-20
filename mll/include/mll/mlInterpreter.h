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

template <typename T>
	std::type_info* get_type_info() {return (std::type_info*)&(typeid(T));}

class mlInterpreter
{
	protected:
		std::vector<std::string> path;
		std::string search_file(std::string filename);
		std::ostream *iout, *ierr;
		std::istream* iin;
		mlLib* owner;
		static std::map<std::type_info*,mlVariable> class_registry;
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
		
		virtual mlVariable get_function_context() {};
		
		bool operator ==(void* id) { return (get_id() == id); }
		
		mlVariable callback(mlVariable callbackKey, std::vector<mlVariable>& args);
		void callback_register(mlVariable key, mlVariable callback);
		
		template <class T,typename... Args>
		static mlVariable constructor_callback(Args...args)
		{
			mlVariable obj;
			obj["_binary_data"] = new T(args...);
			obj.uinion(class_registry[(std::type_info*)(&typeid(T))]);
			return obj;
		}

		
		template <typename T,typename... Args>
			mlVariable class_register(std::string name)
			{
				mlVariable class_var;
				set(name,constructor_callback<T,Args...>);
				class_registry[get_type_info<T>()] = class_var;
				return class_var;
			}
		
		template <typename T,typename R, typename... Args>
			mlVariable method_register(std::string name, R (T::*function)(Args...))
			{
				mlVariable callback(function);
				set(name,callback);
				class_registry[get_type_info<T>()][name] = callback;
			}
};
#endif
