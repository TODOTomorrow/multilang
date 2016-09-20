#ifndef __MLVARIABLE_INC_H__
#define __MLVARIABLE_INC_H__

template <typename T>
	T metatype_t::get()
	{
		return *((T*)data);
	}

template <typename T>
	void metatype_t::set(T obj)
	{
		if (_size != sizeof(obj) && _size != 0)
		{
			free(data);
			data = malloc(sizeof(obj));
		}
		memcpy(data,&obj,sizeof(obj));
	}

template <typename T>
	void mlVariable::set_data(T val)
	{
		data_size = sizeof(val);
		data = new T; 
		*(T*)data = val; 
		type_info = (std::type_info*)&typeid(val);
	}

template <typename... Args,typename T>
	mlVariable::mlVariable(T (*function_pointer)(Args...))
	{
		callback_register(function_pointer);
		type = FUNCTION;
		subtype = ST_CFUNCTION;
		set_data(function_pointer);
	}
	
template <typename... Args,typename Class, typename T>
	mlVariable::mlVariable(T (Class::*function_pointer)(Args...))
		{
			callback_register(function_pointer);
			type = FUNCTION;
			subtype = ST_CFUNCTION;
			set_data(function_pointer);
		}
template <typename T>
	static std::string get_typename(T var) 
	{
		int status; 
		return std::string(abi::__cxa_demangle(typeid(var).name(), 0, 0, &status));
	}

template <typename T>
	T mlVariable::get()
	{
		if (type == NILL && is_auto)
		{
			T var;
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
	T mlVariable::cast()
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
template <typename T>
	mlVariable& mlVariable::operator =(T s) {*(this) = mlVariable(s);}
	
template <typename T>
	mlVariable::operator T() {return get<T>();}

template <typename Arg, typename ...Args>
	void mlVariable::unpack_args(std::vector<mlVariable>& result, Arg arg,Args... args)
	{
		result.push_back(arg);
		unpack_args(result,args...);
	}

template <typename ...Args>
	void mlVariable::operator () (Args...args)
	{
		std::vector<mlVariable> unpacked_args;
		unpack_args(unpacked_args,args...);
		(*this)(unpacked_args);
	}
	
template <typename R, typename ...Args>
	R mlVariable::operator () (Args...args)
	{
		std::vector<mlVariable> unpacked_args;
		unpack_args(unpack_args,args...);
		mlVariable result = (*this)(unpacked_args);
		return result.get<R>();
	}
#endif
