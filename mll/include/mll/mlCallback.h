#ifndef __MLCALLBACK_H__
#define __MLCALLBACK_H__
#include <vector>
#include <mll/mlVariable.h>
#include <map>
#include <thread>
#include <map>

class mlInterpreter;
struct mlCallback;
typedef mlVariable (*callback_helper_function_t)(std::vector<mlVariable> argv,mlCallback* self);
typedef mlVariable (*callback_class_helper_function_t)(void* context, std::vector<mlVariable> argv,mlCallback* self);
void* get_function_context(mlInterpreter* runtime);


class mlCallback
{
	private:
		void* callback_function;
		int size;
		
	public:
		callback_helper_function_t callback_helper_function;
		mlCallback(const mlCallback& cb)
		{
			this->size = cb.size;
			this->callback_helper_function = cb.callback_helper_function;
			if (this->size != 0)
			{
				this->callback_function = malloc(this->size);
				memcpy(this->callback_function,cb.callback_function,this->size);
			}
		}
		template <typename T>
			T get() {return *((T*)callback_function);}
		template <typename T>
			void set(T value) 
			{
				if (callback_function != NULL) 
				{
					free(callback_function); 
					callback_function = NULL;
				}
				
				callback_function = malloc(sizeof(T));
				size = sizeof(T); 
				memcpy(callback_function,&value,sizeof(T));
			}
		
		mlCallback()
		{
			callback_function = NULL;
		}
		~mlCallback()
		{
			if (callback_function != NULL)
				free(callback_function);
		}
		mlCallback operator =(const mlCallback& cb)
		{
			this->size = cb.size;
			this->callback_helper_function = cb.callback_helper_function;
			if (this->size != 0)
			{
				this->callback_function = malloc(this->size);
				memcpy(this->callback_function,cb.callback_function,this->size);
			}
			return *this;
		}
};

class mlCallback_table
{
	std::vector<std::pair<mlVariable,mlVariable>> callbacks;
	public:
		mlVariable* get(mlVariable key) 
		{
			for (int i=0;i<callbacks.size();i++) 
			{
				if (key == callbacks[i].first) 
					return &(callbacks[i].second); 
			}
			return NULL;
		}
		mlVariable* get(int idx)        {if (size() < idx) return NULL; else  return &(callbacks[idx].second);}
		mlVariable* get_key(int idx)        {if (size() < idx) return NULL; else  return &(callbacks[idx].first);}
		void set(mlVariable key, mlVariable cbk);
		bool exists(mlVariable key) {if (get(key) == NULL) return false; return true;}
		int size() {return callbacks.size();}
		void dump()
		{
			for (int i=0;i<callbacks.size();i++)
			{
				std::pair<mlVariable,mlVariable> cb_pair = callbacks[i];
				std::cout << "....[ " << i << "]. Key : " << cb_pair.first << "\n....Value: " << cb_pair.second << "\n" << std::endl;
			}
		}
};

template< std::size_t... Ns >
	struct indices {
	    typedef indices< Ns..., sizeof...( Ns ) > next;
	};
	
template< std::size_t N >
	struct make_indices {
	    typedef typename make_indices< N - 1 >::type::next type;
	};

template<>
	struct make_indices< 0 > {
		typedef indices<> type;
	};

template <typename R, std::size_t N, typename... Args>
	class function_caller
	{
		public:
		template <std::size_t... Is>
			R _call_helper(void* fp, std::vector<mlVariable> args, indices<Is...>)
			{
				typedef R (*callback_func_t)(Args...);
				callback_func_t func = (callback_func_t)(fp);
				return func(args[Is]...);
			}
		
			mlVariable _call(void* func, std::vector<mlVariable> args)
			{
				return mlVariable(_call_helper(func, args, typename make_indices<N>::type()));
			}
	};

/* Specialization for void return functions */
template <std::size_t N, typename... Args>
	class function_caller<void,N,Args...>
	{
		public:
		template <std::size_t... Is>
			void _call_helper(void* fp, std::vector<mlVariable> args, indices<Is...>)
			{
				typedef void (*callback_func_t)(Args...);
				callback_func_t func = (callback_func_t)(fp);
				func(args[Is]...);
			}
		
			mlVariable _call(void* func, std::vector<mlVariable> args)
			{
				_call_helper(func, args, typename make_indices<N>::type());
				return mlVariable();
			}
	};

template <typename Class, typename R, std::size_t N, typename... Args>
	class function_class_caller
	{
		public:
		template <std::size_t... Is>
			void _call_helper(R (Class::*fp)(Args...), Class* obj, std::vector<mlVariable> args, indices<Is...>)
			{
				typedef void (Class::*callback_func_t)(Args...);
				(obj->*fp)(args[Is]...);
			}
		
			mlVariable _call( R (Class::*func)(Args...), Class* obj, std::vector<mlVariable> args)
			{
				_call_helper(func, obj, args, typename make_indices<N>::type());
				return mlVariable();
			}
	};

class mlCallbackMgr
{	
	protected:
		static mlCallback_table callback_table;
		static std::map<std::thread::id,mlInterpreter*> context_table;
		static mlInterpreter* get_context() 
		{
			if (context_table.count(std::this_thread::get_id()) == 0) return NULL; 
			return context_table[std::this_thread::get_id()];
		}
		
		
		
		template <typename R, typename... Args>
			static mlVariable callback_helper(std::vector<mlVariable> argv, mlCallback* self)
			{
				std::tuple<Args...> tuple;
				if (argv.size() < (sizeof...(Args)))
				{
					
					for (int i=0;i<(sizeof...(Args)) - argv.size();i++)
					{
						mlVariable var;
						var.set_auto();
						argv.push_back(var);
					}
				}
				
				function_caller<R,sizeof...(Args),Args...> caller;
				return caller._call(self->get<void*>(),argv);
			}
			
		template <typename Class, typename R, typename... Args>
			static mlVariable callback_class_helper(Class* context, std::vector<mlVariable> argv, mlCallback* self)
			{
				std::tuple<Args...> tuple;
				if (argv.size() < (sizeof...(Args)))
				{
					
					for (int i=0;i<(sizeof...(Args)) - argv.size();i++)
					{
						mlVariable var;
						var.set_auto();
						argv.push_back(var);
					}
				}
				
				function_class_caller<Class,R,sizeof...(Args),Args...> caller;
				return caller._call(self->get<R (Class::*)(Args...)>(), context, argv);
			}
			
			template <typename R>
			static mlVariable callback_helper_raw(std::vector<mlVariable> argv, mlCallback* self)
			{
				function_caller<R,1,std::vector<mlVariable>> caller;
				std::vector<mlVariable> args;
				args.push_back(argv);
				return caller._call(self->get<void*>(),args);
			}
			
	public:
		static metatype_t C_CALLBACK_TYPE;
		static metatype_t CLASS_CALLBACK_TYPE;
		
		template <typename T>
		static mlVariable call(T function_ptr,std::vector<mlVariable> args, mlInterpreter* context)
		{
			
			mlVariable ptr(function_ptr);
			if (!callback_table.exists(ptr))
			{
				return mlVariable();
			}
			
			mlVariable* cbptr = callback_table.get(ptr);
			if (cbptr == NULL)
				mlException("Called undefined callback");
			
			mlVariable cb = *cbptr;
			context_table[std::this_thread::get_id()] = context;
			
			if (cb.get_metatype() == C_CALLBACK_TYPE)
			{
				std::cout << "C callback" << std::endl;
				mlCallback callback = cb;
				return callback.callback_helper_function(args,&callback);
			}
			else if (cb.get_metatype() == CLASS_CALLBACK_TYPE)
			{
				std::cout << "C++ callback" << std::endl;
				mlCallback callback = cb;
				void* func_ctx = get_function_context(context);
				return ((callback_class_helper_function_t)(callback.callback_helper_function))(func_ctx, args,&callback);
			}
			else
			{
				return cb(args,context);
			}
		}
		
		/* Register callback to object member function */
		template <typename Class, typename... Args,typename T>
			static void callback_register(T (Class::*function_pointer)(Args...))
			{
				/* We need some uniquie pointer to associate with function */
				mlVariable key((void*)(&function_pointer));
				callback_register(key,function_pointer);
			}
		
		template <typename... Args,typename T>
			static void callback_register(T (*function_pointer)(Args...))
			{
				mlVariable key((void*)function_pointer);
				callback_register(key,function_pointer);
			}
		
		/* Template to raw functions, that get raw vector of mlVariables */
		template <typename T>
			static void callback_register(mlVariable key, T (*function_pointer)(std::vector<mlVariable>))
			{
				mlCallback callback;
				callback.set(function_pointer);
				callback.callback_helper_function = &mlCallbackMgr::callback_helper_raw<T>;
				mlVariable callback_wrap(callback);
				callback_wrap.set_metatype(C_CALLBACK_TYPE);
				callback_table.set(key,callback_wrap);
			}
		
		template <typename Class, typename... Args,typename T>
			static void callback_register(mlVariable key, T (Class::*function_pointer)(Args...))
			{
				mlCallback callback;
				callback.set(function_pointer);
				callback.callback_helper_function = (callback_helper_function_t)(&mlCallbackMgr::callback_class_helper<Class, T, Args...>);
				mlVariable callback_wrap(callback);
				callback_wrap.set_metatype(CLASS_CALLBACK_TYPE);
				
				callback_table.set(key,callback_wrap);
			}
		
		template <typename... Args,typename T>
			static void callback_register(mlVariable key, T (*function_pointer)(Args...))
			{
				mlCallback callback;
				callback.set(function_pointer);
				callback.callback_helper_function = &mlCallbackMgr::callback_helper<T, Args...>;
				mlVariable callback_wrap(callback);
				
				callback_wrap.set_metatype(C_CALLBACK_TYPE);
				callback_table.set(key,callback_wrap);
			}
		static void callback_register(mlVariable key, mlVariable value);
};

template <typename... Args,typename T>
	void callback_register(T (*function_pointer)(Args...))
	{
		mlCallbackMgr::callback_register(function_pointer);
	}

template <typename Class, typename... Args,typename T>
	static void callback_register(T (Class::*function_pointer)(Args...))
	{
		mlCallbackMgr::callback_register(function_pointer);
	}

#endif
