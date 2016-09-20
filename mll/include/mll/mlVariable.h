#ifndef __MLVARIABLE_H__
#define __MLVARIABLE_H__
#include <map>
#include <vector>
#include <iostream>
#include <mll/mlException.h>
#include <set>
#include <typeinfo>
#include <cxxabi.h>

extern "C" {
	#include <string.h>
	#include <malloc.h>
}


class mlInterpreter;
class mlCallbackContext;
template <typename... Args,typename T>
	static void callback_register(T (*function_pointer)(Args...));
template <typename Class, typename... Args,typename T>
	static void callback_register(T (Class::*function_pointer)(Args...));

#define MAX_TYPE_ID  9999999
class metatype_t
{
	private:
		void* data;
		int _size;
		int id;
		static std::vector<metatype_t> user_types;
	public:
		static metatype_t generate(void* metadata_buf, int size);
		static metatype_t* find(int id);
		
	template <typename T>
		static metatype_t generate(T data) {generate((void*)&data,sizeof(data));}
		static metatype_t generate() {return generate(NULL, 0);}
		static metatype_t* get(int id);
		int size() {return _size;}
		int get_id() {return id;}
		metatype_t(const metatype_t& obj);
		metatype_t();
		~metatype_t()
		{
			if (data != NULL)
				free(data);
		}
		
	template <typename T>
		void set(T obj);
	template <typename T>
		T    get();
		
	bool operator ==(metatype_t compare);
	bool operator !=(metatype_t compare) {return (this->id != compare.id);}
	friend std::ostream& operator <<(std::ostream& str, const metatype_t& mt);
	static bool is_valid(metatype_t* mtype)  {if (metatype_t::get(mtype->id) == NULL) return false; else return true;}
};
extern metatype_t default_metatype;

class mlVariable
{
	public:
		enum type_t  
		{
			STRING, 
			NUMBER, 
			BOOLEAN, 
			OBJECT, 
			FUNCTION, 
			NILL, 
			CLASS, 
			NONE
		};
		
		enum subtype_t 
		{
			ST_CSTRING, 
			ST_CFUNCTION, 
			ST_CFUNCTION_RVOID, 
			ST_CFUNCTION_RONE, 
			ST_CFUNCTION_FN_RONE, 
			ST_NONE, 
			ST_BINARY
		};
		
	private:
		std::type_info* type_info = NULL;
		void* data;
		int data_size;
		type_t type;
		subtype_t subtype;
		mlInterpreter* owner;
		std::map<std::string,mlVariable*> fields;
		bool is_auto = false;
		metatype_t metatype = default_metatype;
		
	template <typename T> 
		void set_data(T val);
		void set_data(void* ptr, int size);
		
		void free_mem();
		void to_nil();
		
	public:
		metatype_t  get_metatype();
		type_t get_type()       {return type;}
		subtype_t get_subtype() {return subtype;}
		int size()              {return data_size;}
		void* get_data_buf()    {return data;}
		
		void set_metatype(const metatype_t& t);
		void set_owner(mlInterpreter* own);
		void set_auto() {is_auto = true;}
		
		mlVariable(const mlVariable& obj);
		mlVariable(const std::string& val) {type = STRING;   subtype = ST_NONE;     set_data(val); }
		mlVariable(const char* val)        {type = STRING;   subtype = ST_CSTRING;  set_data((char*)val); }
		mlVariable(int val)                {type = NUMBER;   subtype = ST_NONE;     set_data(val); }
		mlVariable(bool val)               {type = BOOLEAN;  subtype = ST_NONE;     set_data(val); }
		mlVariable(void* val)              {type = FUNCTION; subtype = ST_NONE;     set_data(val); }
		mlVariable(void* ptr, int size)    {type = OBJECT;   subtype = ST_BINARY;   set_data(ptr,size);}
		mlVariable()                       {type = NILL;     subtype = ST_NONE; data = NULL; data_size = 0;}
	template <typename T>
		mlVariable(T val)                  { type = OBJECT;   subtype = ST_BINARY;    set_data(val);}		
	template <typename... Args,typename T>
		mlVariable(T (*function_pointer)(Args...));
	template <typename... Args,typename Class, typename T>
		mlVariable(T (Class::*function_pointer)(Args...));
		~mlVariable();
		
		
		std::string get_typename();
	template <typename T>
		T get();	
	template <typename T>
		T cast();
		
		void uinion(mlVariable v);
		std::vector<std::string> keys();
		
		/* Cast operators */
		template <typename T>
			mlVariable& operator =(T s);
		template <typename T>
			operator T();
		operator std::string();
		
		/* Operators */
		mlVariable& operator [](std::string name);
		mlVariable& operator =(const mlVariable& v);
		friend std::ostream& operator <<(std::ostream& str,mlVariable v);
		mlVariable operator ()(std::vector<mlVariable> args, mlInterpreter* context = NULL);
		
		void unpack_args(std::vector<mlVariable>& result) {}	
	template <typename Arg, typename ...Args>
		void unpack_args(std::vector<mlVariable>& result, Arg arg,Args... args);
		
	template <typename ...Args>
		void operator () (Args...args);
	template <typename R, typename ...Args>
		R operator () (Args...args);
	
		bool operator ==(const mlVariable& comp);
		bool operator < ( mlVariable const& rhs ) const { return memcmp(this,&rhs,sizeof(mlVariable)); }
		
	friend mlInterpreter;
};
#include <mll/mlVariable.inc>

#endif
