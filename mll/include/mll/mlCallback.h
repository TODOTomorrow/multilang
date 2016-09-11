#ifndef __MLCALLBACK_H__
#define __MLCALLBACK_H__
#include <vector>
#include <mll/mlVariable.h>
#include <map>

struct mlCallback
{
	mlVariable (*callback_helper_function)(std::vector<mlVariable> argv,mlCallback* self);
	void* callback_function;
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
				func(args[Is]...);
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

class mlCallbackMgr
{	
	protected:
		static mlCallback_table callback_table;
		
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
				return caller._call(self->callback_function,argv);
			}

	public:
		class TypesHelper
		{
			public:
				metatype_t* C_CALLBACK_TYPE;
				TypesHelper()
				{
					C_CALLBACK_TYPE = mlVariable::create_metatype();
				}
		};
		static TypesHelper Types;
		
		template <typename T>
		static mlVariable call(T function_ptr,std::vector<mlVariable> args)
		{
			
			mlVariable ptr(function_ptr);
			if (!callback_table.exists(ptr))
				return mlVariable();
			mlVariable* cbptr = callback_table.get(ptr);
			if (cbptr == NULL)
				mlException("Called undefined callback");
			
			mlVariable cb = *cbptr;
			if (cb.get_metatype() == *(Types.C_CALLBACK_TYPE))
			{
				mlCallback callback = cb;
				return callback.callback_helper_function(args,&callback);
			}
			else
			{
				return cb(args);
			}
		}
		
		template <typename... Args,typename T>
			static void callback_register(T (*function_pointer)(Args...))
			{
				mlVariable key((void*)function_pointer);
				callback_register(key,function_pointer);
			}
		template <typename... Args,typename T>
			static void callback_register(mlVariable key, T (*function_pointer)(Args...))
			{
				mlCallback callback;
				callback.callback_function = (void*)(function_pointer);
				callback.callback_helper_function = &mlCallbackMgr::callback_helper<T, Args...>;
				mlVariable callback_wrap(callback);
				callback_wrap.set_metatype(Types.C_CALLBACK_TYPE);
				callback_table.set(key,callback_wrap);
			}
		static void callback_register(mlVariable key, mlVariable value);
};

template <typename... Args,typename T>
	void callback_register(T (*function_pointer)(Args...))
	{
		mlCallbackMgr::callback_register(function_pointer);
	}
	
#endif
