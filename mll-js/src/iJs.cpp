#include <iostream>
#include <fstream>
#include <iJs.h>
#include <algorithm>

int js_callback(duk_context* ctx)
{
	duk_c_function funcptr;
	std::vector<mlVariable> args;
	iJs* runtime = (iJs*)mlFactory::search((void*)ctx);
	if (runtime == NULL)
	{
		std::cerr << "Cannot find runtime for callback" << std::endl;
		return 0;
	}
	
	int cnt = duk_get_top(ctx);
	for (int i=0;i<cnt;i++)
		args.push_back(runtime->get_from_stack(-1));
	std::reverse(args.begin(),args.end());
	
	duk_push_current_function(ctx);
	mlVariable v = mlVariable(duk_get_c_function(ctx,1));
	duk_pop(ctx);
	
	duk_push_current_function(ctx);
	mlVariable function_desc =  runtime->get_from_stack(-1,DUK_TYPE_OBJECT);
	mlVariable result = runtime->callback(function_desc["_function_name"],args);
	runtime->push_to_stack(result);
	return 1;
}

mlVariable iJs::get_function_context()
{
	duk_push_current_function(ctx);
	return get_from_stack(-1,DUK_TYPE_OBJECT);
}

iJs* mllib_generate()
{
	return new iJs();
}

iJs::iJs()
{
	this->ctx = duk_create_heap_default();
	if (this->ctx == NULL)
		*ierr << "Error while creating Javascript context" << std::endl;
}

bool iJs::exec(std::string str)
{
	str = search_file(str);
	if (duk_peval_file(ctx,str.c_str()) != 0)
	{
		*ierr << "Error : " << duk_safe_to_string(ctx, -1) << std::endl;
		return false;
	}
	return true;
}

bool iJs::eval(std::string str)
{
	if (duk_peval_string(ctx,str.c_str()))
	{
		*ierr << "Error : " << duk_safe_to_string(ctx, -1) << std::endl;
		return false;
	}
	return true;
}


void iJs::push_to_stack(mlVariable val)
{
	std::vector <std::string> keys;
	void* ptr;
	switch (val.get_type())
	{
		case mlVariable::STRING:
			if (val.get_subtype() == mlVariable::ST_NONE)
				duk_push_string(ctx, (val.get<std::string>()).c_str());
			else
				duk_push_string(ctx, val.get<char*>());
			break;
		case mlVariable::BOOLEAN:
			duk_push_boolean(ctx, val.get<bool>());
			break;
		case mlVariable::NUMBER:
			duk_push_number(ctx, val.get<int>());
			break;
		case mlVariable::NILL:
			duk_push_null(ctx);
			break;
		case mlVariable::FUNCTION:
			duk_push_c_function(ctx,js_callback,DUK_VARARGS);
			break;
		case mlVariable::OBJECT:
			if (val.get_subtype() != mlVariable::ST_BINARY)
			{
				duk_push_object(ctx);
				keys = val.keys();
				for (int i=0;i<keys.size();i++)
				{
					duk_push_string(ctx,keys[i].c_str());
					
					if (val[keys[i]].get_type() == mlVariable::FUNCTION)
						callback_register(val[keys[i]],val[keys[i]]);
						
					push_to_stack(val[keys[i]]);
					
					if (val[keys[i]].get_type() == mlVariable::FUNCTION)
					{
						duk_push_string(ctx,keys[i].c_str());
						duk_put_prop_string(ctx,-2,"_function_name");
					}
					
					duk_put_prop(ctx,-3);
					
				}
			}
			else
			{
				ptr = duk_push_buffer_raw(ctx, val.size(), 0);
				memcpy(ptr,val.get_data_buf(),val.size());
			}
			break;
		case mlVariable::CLASS:
				ptr = duk_push_buffer_raw(ctx, val.size(), 0);
				memcpy(ptr,val.get_data_buf(),val.size());
			break;
		default:
			break;
	}
}

mlVariable  iJs::get_from_stack(int idx, int type)
{
	mlVariable result;
	void* ptr;
	int size; 
	if ( type < 0 ) 
		type = duk_get_type(ctx,idx);
	switch (type)
	{
		case DUK_TYPE_BOOLEAN:
			result = mlVariable((bool)duk_get_boolean(ctx,idx));
			duk_pop(ctx);
			return result;
		
		case DUK_TYPE_BUFFER:
			ptr = duk_get_buffer(ctx,idx,(duk_size_t*)&size);
			duk_pop(ctx);
			return mlVariable(ptr,size);
			
		case DUK_TYPE_UNDEFINED:
			return mlVariable();
			
		case DUK_TYPE_STRING:
			result = mlVariable(duk_get_string(ctx,idx));
			duk_pop(ctx);
			return result;
			
		case DUK_TYPE_OBJECT:
			duk_enum(ctx,idx, DUK_ENUM_OWN_PROPERTIES_ONLY);//FIXME?
			
			while (duk_next(ctx,-1,1))
			{
				char* name = (char*)duk_to_string(ctx,-2);
				result[name] = get_from_stack(-1);
				duk_pop(ctx);
			}
			return result;
			
		case DUK_TYPE_NUMBER:
			result = mlVariable((int)duk_get_number(ctx,idx));
			duk_pop(ctx);
			return result;
			
		case DUK_TYPE_NULL:
			duk_pop(ctx);
			return mlVariable();
			
		case DUK_TYPE_POINTER:
			*ierr << "LIGHT FUNC" << std::endl;
			duk_pop(ctx);
			return mlVariable();
			
		case DUK_TYPE_LIGHTFUNC:
			*ierr << "LIGHT FUNC" << std::endl;
			duk_pop(ctx);
			return mlVariable();
			
		default:
			*ierr << "unknown type" << duk_get_type(ctx,idx) << std::endl;
			return mlVariable();
	}
}

bool iJs::set(std::string name, mlVariable val)
{
	duk_push_global_object(ctx);
	duk_push_string(ctx,name.c_str());
	push_to_stack(val);
	
	if (val.get_type() == mlVariable::FUNCTION)
	{
		callback_register(name,val);
		duk_push_string(ctx,name.c_str());
		duk_put_prop_string(ctx,-2,"_function_name");
	}
	
	duk_put_prop(ctx,-3);
}


mlVariable  iJs::get(std::string name)
{
	duk_push_global_object(ctx);
	duk_get_prop_string(ctx,-1,name.c_str());
	
	return get_from_stack(-1);
}
