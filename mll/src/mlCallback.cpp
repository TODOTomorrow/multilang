#include <mll/mlCallback.h>
#include <mll/mlInterpreter.h>

mlCallback_table mlCallbackMgr::callback_table;
metatype_t mlCallbackMgr::C_CALLBACK_TYPE = metatype_t::generate();
metatype_t mlCallbackMgr::CLASS_CALLBACK_TYPE  = metatype_t::generate();
metatype_t default_metatype = metatype_t::generate();
std::map<std::thread::id,mlInterpreter*> mlCallbackMgr::context_table;


void mlCallback_table::set(mlVariable key, mlVariable cbk) 
{
	metatype_t mt = cbk.get_metatype();
	if (cbk.get_metatype() == default_metatype)
	{
		cbk.set_metatype(mlCallbackMgr::C_CALLBACK_TYPE);
	}
	return;
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
			if ((v.get_metatype() == C_CALLBACK_TYPE) || 
			    (v.get_metatype() == CLASS_CALLBACK_TYPE))
			{
				mlCallback cb = v.get<mlCallback>();
				if (cb.get<void*>() == value.cast<void*>())
				{
					callback_table.set(key,cb);
					return;
				}
			}
		}
	}
	callback_table.set(key,value);
}
