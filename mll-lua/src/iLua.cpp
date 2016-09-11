#include <mll/mlFactory.h>
#include <iLua.h>

int lua_callback(lua_State *L)
{
	std::vector<mlVariable> args;
	iLua* runtime = (iLua*)mlFactory::search((void*)L);
	if (runtime == NULL)
	{
		std::cerr << "Cannot find runtime for callback" << std::endl;
		return 0;
	}
	for (int i=1;i<=lua_gettop(L);i++)
		args.push_back(runtime->get_from_stack(i));
	
	lua_Debug info = runtime->get_debug_info();
	std::string function_name(info.name);
	mlVariable func_desc;
	func_desc["_function_name"] = function_name;
	mlVariable ret = runtime->callback(function_name,args);
	
	runtime->push_to_stack(ret);
	return 1;
}

void iLua::push_to_stack(mlVariable val)
{
	std::vector <std::string> keys;
	switch (val.get_type())
	{
		case mlVariable::STRING:
			if (val.get_subtype() == mlVariable::ST_NONE)
				lua_pushstring(ctx, (val.get<std::string>()).c_str());
			else
				lua_pushstring(ctx, val.get<char*>());
			break;
		case mlVariable::BOOLEAN:
			lua_pushboolean(ctx, val.get<bool>());
			break;
		case mlVariable::NUMBER:
			lua_pushinteger(ctx, val.get<int>());
			break;
		case mlVariable::NILL:
			lua_pushnil(ctx);
			break;
		case mlVariable::FUNCTION:
			lua_pushcfunction(ctx,lua_callback);
			break;
		case mlVariable::OBJECT:
			if (val.get_subtype() != mlVariable::ST_BINARY)
			{
				lua_newtable(ctx);
				keys = val.keys();
				for (int i=0;i<keys.size();i++)
				{
					lua_pushstring(ctx,keys[i].c_str());
					if (val[keys[i]].get_type() == mlVariable::FUNCTION)
						callback_register(keys[i],val[keys[i]]);
					push_to_stack(val[keys[i]]);
					lua_settable(ctx,-3);
				}
			}
			else
			{
				void* ptr = lua_newuserdata(ctx,val.size());
				memcpy(ptr,val.get_data_buf(),val.size());
			}
			break;
		default:
			break;
	}
}

lua_Debug	iLua::get_debug_info()
{
	lua_Debug info;
	if (!lua_getstack(ctx, 0, &info))
		return info;
	
	lua_getinfo(ctx,"nSl", &info);
	if (info.name == NULL)
		return info;
	
	return info;
}

iLua::iLua()
{
	ctx = luaL_newstate();
	add_lib("base"     , luaopen_base);
	add_lib("io"       , luaopen_io);
	add_lib("table"    , luaopen_table);
	add_lib("os"       , luaopen_os);
	add_lib("string"   , luaopen_string);
	add_lib("coroutine", luaopen_coroutine);
	add_lib("bit32"    , luaopen_bit32);
	add_lib("math"     , luaopen_math);
	add_lib("debug"    , luaopen_debug);
	add_lib("package"  , luaopen_package);
}

void iLua::add_lib(std::string name, lua_CFunction func)
{
	luaL_requiref(ctx, name.c_str(), func, 1);
	lua_settop(ctx, 0);
}

iLua* mllib_generate()
{
	return new iLua();
}

bool iLua::eval(std::string str)
{
	if (luaL_dostring(ctx, str.c_str()) != 0)
	{
		*ierr << lua_tostring(ctx, -1) << std::endl;
		return false;
	}
    return true;
}


bool iLua::exec(std::string filename)
{
	filename = search_file(filename);
	if (luaL_dofile(ctx, filename.c_str()) != 0)
	{
		*ierr << lua_tostring(ctx, -1) << std::endl;
		return false;
	}
	return true;	
}

bool iLua::set(std::string name, mlVariable val)
{
	if (val.get_type() == mlVariable::FUNCTION)
		callback_register(name,val);
	push_to_stack(val);
	lua_setglobal(ctx , name.c_str());
}

mlVariable  iLua::get_from_stack(int idx)
{
	mlVariable retVal;
	const char* name;
	
	switch (lua_type(ctx,idx))
	{
		case LUA_TNIL:
			return mlVariable();
		case LUA_TBOOLEAN:
			return mlVariable((bool)lua_toboolean(ctx,idx));
		case LUA_TNUMBER:
			return (int)lua_tonumber(ctx,idx);
		case LUA_TSTRING:
			return mlVariable(std::string(lua_tostring(ctx,idx)));
		case LUA_TTABLE:
			lua_pushvalue(ctx, idx);
			lua_pushnil(ctx);
			while (lua_next(ctx, -2))
			{
				lua_pushvalue(ctx,-2);
				name = lua_tostring(ctx,-1);
				retVal[name] =  get_from_stack(-2);
				lua_pop(ctx,2);
			}
			lua_pop(ctx,1);
			return retVal;
		case LUA_TFUNCTION:
			return mlVariable();
//			return mlVariable(luaL_ref(ctx, LUA_REGISTRYINDEX) , mlVariable::FUNCTION);
		case LUA_TUSERDATA:
			break;
		case LUA_TTHREAD:
			break;
		default:
			*ierr << "Cannot find global variable  by id - " << idx << std::endl;
			return mlVariable();
	}
}

mlVariable  iLua::get(std::string name)
{
	lua_getglobal(ctx,name.c_str());
	return get_from_stack(-1);
}

std::vector<mlVariable> iLua::call(std::string functionname,std::vector<mlVariable>& args)
{
	std::vector <mlVariable> result;
	lua_getglobal(ctx,functionname.c_str());
	if (!lua_isfunction(ctx,-1))
	{
		*ierr << "Error : cannot get function by reference";
		return result;
	}
	for (int i=0;i<args.size();i++)
		push_to_stack(args[i]);
	lua_call(ctx,args.size(), 1);
	result.push_back(get_from_stack(1));
	return result;
}

std::vector<mlVariable> iLua::call(mlVariable func,std::vector<mlVariable>& args)
{
	std::vector <mlVariable> result;
	if (func.get_type() != mlVariable::FUNCTION)
	{
		*ierr << "Error : cannot call not LUA function";
		return result;
	}
	int refid = func.get<int>();
	lua_rawgeti(ctx, refid, LUA_REGISTRYINDEX);
	if (!lua_isfunction(ctx,-1))
	{
		*ierr << "Error : cannot get function by reference";
		return result;
	}
	for (int i=0;i<args.size();i++)
		push_to_stack(args[i]);
	lua_call(ctx,args.size(), 1);
	result.push_back(get_from_stack(-1));
	return result;
}
