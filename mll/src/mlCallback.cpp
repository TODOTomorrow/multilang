#include <mll/mlCallback.h>
#include <mll/mlInterpreter.h>
mlCallbackMgr::TypesHelper mlCallbackMgr::Types;

void mlCallback_table::set(mlVariable key, mlVariable cbk) 
{
	cbk.set_metatype(mlCallbackMgr::Types.C_CALLBACK_TYPE);
	if (get(key) == NULL)
		callbacks.push_back(std::pair<mlVariable,mlVariable>(key,cbk));
}

void mlCallbackMgr::callback_register(mlVariable key, mlVariable value)
{
	/* If is C function. Search it in current callback table and generate alias for it */
	if (value.get_type() == mlVariable::FUNCTION && 
	    value.get_subtype() == mlVariable::ST_CFUNCTION)
	{
		for (int i=0;i<callback_table.size();i++)
		{
			mlVariable v = *(callback_table.get(i));
			if (v.get_metatype() == *(Types.C_CALLBACK_TYPE))
			{
				mlCallback cb = v.get<mlCallback>();
				if (cb.callback_function == value.cast<void*>())
				{
					callback_table.set(key,cb);
					return;
				}
			}
		}
	}
	callback_table.set(key,value);
}
