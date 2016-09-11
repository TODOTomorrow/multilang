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

#define MAX_TYPE_ID  9999999
typedef struct 
{
	void* data;
	int size;
	int id;
} metatype_t;
bool operator ==(metatype_t t1,metatype_t t2);
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
		
		template <typename T>
			void set_data(T val)
		{
			data_size = sizeof(val);
			data = new T; 
			*(T*)data = val; 
			type_info = (std::type_info*)&typeid(val);
		}
		
		void set_data(void* ptr, int size)
		{
			data_size = size;
			data = malloc(size); 
			memcpy(data,ptr,size);
			type_info = (std::type_info*)&typeid(void*);
		}
		

		std::map<std::string,mlVariable*> fields;
		
		/* FIXME */
		void free_mem() 
		{
			if (this->data == 0) return; 
			free(data); 
		}
		void to_nil() {free(data); data_size = 0; type = NILL; subtype = ST_NONE; type_info = NULL;}//FIXME add gc
		
		bool is_auto = false;
		metatype_t* metatype = NULL;
		static std::vector<metatype_t> user_types;
	public:
		static metatype_t* create_metatype(void* metadata_buf, int size);
		static metatype_t* create_metatype() {return create_metatype(NULL, 0);}
		static metatype_t* find_metatype(int id);
		metatype_t  get_metatype()            {if (metatype == NULL) return default_metatype; else return *metatype;}
		void set_metatype(metatype_t* t) {metatype = new metatype_t(); memcpy(metatype,t,sizeof(metatype_t));}
		
		mlVariable(const mlVariable& obj)
		{
			type = obj.type;
			subtype = obj.subtype;
			if (obj.data_size)
			{
				data = malloc(obj.data_size);
				memcpy(data,obj.data,obj.data_size);
			}
			data_size = obj.data_size;
			fields = obj.fields;
			type_info = obj.type_info;
			is_auto = obj.is_auto;
			if (obj.metatype != NULL)
			{
				metatype = new metatype_t();
				memcpy(metatype,obj.metatype,sizeof(metatype_t));
			}
			else
				metatype = NULL;
		}
	
		void set_owner(mlInterpreter* own);
		type_t get_type() {return type;}
		subtype_t get_subtype() {return subtype;}
		int size() {return data_size;}
		void* get_data_buf() {return data;}
		void set_auto() {is_auto = true;}
		~mlVariable();
		mlVariable(const std::string& val) {type = STRING;   subtype = ST_NONE;     set_data(val); }
		mlVariable(const char* val)        {type = STRING;   subtype = ST_CSTRING;  set_data((char*)val); }
		mlVariable(int val)                {type = NUMBER;   subtype = ST_NONE;     set_data(val); }
		mlVariable(bool val)               {type = BOOLEAN;  subtype = ST_NONE;     set_data(val); }
		mlVariable(void* val)              {type = FUNCTION; subtype = ST_NONE;     set_data(val); }
		mlVariable(void* ptr, int size)    {type = OBJECT;   subtype = ST_BINARY;   set_data(ptr,size);}
		mlVariable()                       {type = NILL;     subtype = ST_NONE; data = NULL; data_size = 0;}
		template <typename T>
			mlVariable(T val)              {type = OBJECT; subtype = ST_BINARY; set_data(val);}		
		template <typename... Args,typename T>
			mlVariable(T (*function_pointer)(Args...))
			{
				callback_register(function_pointer);
				type = FUNCTION;
				subtype = ST_CFUNCTION;
				set_data(function_pointer);
			}
		
		template <typename T>
			static std::string get_typename(T var) {int status; return std::string(abi::__cxa_demangle(typeid(var).name(), 0, 0, &status));}
		std::string get_typename() {int status; return std::string(abi::__cxa_demangle(this->type_info->name(), 0, 0, &status));}
		
		template <typename T>
			T get()
			{
				if (type == NILL && is_auto)
				{
					T var;
					memset(&var,0,sizeof(T));
					set_data(var);
				}
				
				if (data == NULL && !is_auto) 
					throw mlException("Error: Attempt to reading NULL value!"); 
				else 
				{
					if (sizeof(T) != data_size)
						throw mlException("Error in values casting( Different sizes )!"); 
					T d = *((T*)(data));
					if (this->type_info != NULL && typeid(d) != *(this->type_info))
					{
						int status;
						throw mlException("Cannot cast '" + std::string(abi::__cxa_demangle(this->type_info->name(), 0, 0, &status)) + 
						"' to '" + std::string(abi::__cxa_demangle(typeid(d).name(), 0, 0, &status)) + "'");
					}
					return d;
				}
			}
		
		template <typename T>
			T cast()
			{
				if (data == NULL && is_auto)
				{
					T var;
					memset(&var,0,sizeof(T));
					set_data(var);
				}
				
				if (data == NULL && !is_auto) 
					throw mlException("Error: Attempt to reading NULL value!"); 
				else 
				{
					if (sizeof(T) != data_size)
						throw mlException("Error in values casting!"); 
					T d = *((T*)(data));
					return d;
				}
			}
		
		std::vector<std::string> keys();
		
		/* Cast operators */
		template <typename T>
			mlVariable& operator =(T s) {*(this) = mlVariable(s);}
		template <typename T>
			operator T() {return get<T>();}
		operator std::string() {if (subtype==ST_CSTRING) return std::string(get<char*>()); else return get<std::string>();}
		
		/* Operators */
		mlVariable& operator [](std::string name);
		mlVariable& operator =(const mlVariable& v);
		friend std::ostream& operator <<(std::ostream& str,mlVariable v);
		std::vector<mlVariable> operator ()(std::vector<mlVariable> args, mlCallbackContext* context);
		mlVariable operator ()(std::vector<mlVariable> args);
		
		void unpack_args(std::vector<mlVariable>& result) {}
		
		template <typename Arg, typename ...Args>
		void unpack_args(std::vector<mlVariable>& result, Arg arg,Args... args)
		{
			result.push_back(arg);
			unpack_args(result,args...);
		}
		
		template <typename ...Args>
			void operator () (Args...args)
			{
				std::vector<mlVariable> unpacked_args;
				unpack_args(unpacked_args,args...);
				(*this)(unpacked_args);
			}
			
		template <typename R, typename ...Args>
			R operator () (Args...args)
			{
				std::vector<mlVariable> unpacked_args;
				unpack_args(unpack_args,args...);
				mlVariable result = (*this)(unpacked_args);
				return result.get<R>();
			}
		
		bool operator ==(const mlVariable& comp);
		bool operator < ( mlVariable const& rhs ) const { return memcmp(this,&rhs,sizeof(mlVariable)); }
		
		friend mlInterpreter;
};
#endif
