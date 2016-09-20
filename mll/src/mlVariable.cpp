#include <algorithm>
#include <mll/mlVariable.h>
#include <mll/mlInterpreter.h>
#include <mll/mlCallback.h>

extern metatype_t default_metatype;
std::vector<metatype_t> metatype_t::user_types;
bool metatype_t::operator ==(metatype_t compar) 
{
	return this->id == compar.id;
}



metatype_t* metatype_t::find(int id)
{
	for (int i=0;i<user_types.size();i++)
		if (user_types[i].id == id) return &(user_types[i]);
	return NULL;
}

metatype_t* metatype_t::get(int id)
{
	for (int i = 0 ; i < user_types.size(); i++)
	{
		if (user_types[i].id == id)
			return &(user_types[i]);
	}
	return NULL;
}


metatype_t metatype_t::generate(void* metadata_buf, int size)
{
	int type_id = 0;
	for (int i = 0 ; i < user_types.size(); i++)
	{
		
		if (user_types[i].id == type_id)
		{
			type_id++;
			if (type_id == MAX_TYPE_ID)
				throw mlException("Error: Max type exceed");
			i = 0;
		}
	}
	
	metatype_t metadata;
	metadata.id = type_id;
	if (size)
	{
		metadata.data = malloc(size);
		memcpy(metadata.data,metadata_buf,size);
	}
	metadata._size = size;
	user_types.push_back(metadata);
	std::cout << &user_types << std::endl;
	for (int i = 0 ; i < user_types.size(); i++)
		std::cout << "\tut [" << i << "] = " << user_types[i] << std::endl;
	return metadata;
}

void mlVariable::to_nil() 
{
	free(data); 
	data_size = 0; 
	type = NILL; 
	subtype = ST_NONE; 
	type_info = NULL;
}

void mlVariable::free_mem() 
{
	if (this->data == 0) return; 
	free(data); 
}

void mlVariable::uinion(mlVariable v)
{
	this->fields = v.fields;
}

std::string mlVariable::get_typename()
{
	int status; 
	return std::string(abi::__cxa_demangle(this->type_info->name(), 0, 0, &status));
}

mlVariable::mlVariable(const mlVariable& obj)
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
	if (((mlVariable&)obj).metatype != default_metatype)
	{
		metatype = obj.metatype;
	}
	else
		metatype = default_metatype;
}

std::ostream& operator <<(std::ostream& str, const metatype_t& mt)
{
	str << "Metatype " << mt.id << ". (size: " << mt._size << " , ptr: " << mt.data << ") ";
	return str;
}


metatype_t::metatype_t()
{
	data = NULL;
	_size = 0;
}

metatype_t::metatype_t(const metatype_t& obj)
{
	std::cout << "Copying metatype. Size: " << ((metatype_t&)obj).size() << " Pointer: " << obj.data << std::endl;
	if (obj._size != 0)
	{
		data = malloc(_size);
		memcpy(data,obj.data,_size);
	}
	_size = obj._size;
	id = obj.id;
}

void mlVariable::set_metatype(const metatype_t& t) 
{
	metatype = t;
}

metatype_t  mlVariable::get_metatype()
{
	return metatype;
}


mlVariable::operator std::string()
{
	if (subtype==ST_CSTRING) 
		return std::string(get<char*>()); 
	else 
		return get<std::string>();
}

void mlVariable::set_data(void* ptr, int size)
{
	data_size = size;
	data = malloc(size); 
	memcpy(data,ptr,size);
	type_info = (std::type_info*)&typeid(void*);
}

void mlVariable::set_owner(mlInterpreter* interp)
{
	this->owner = interp;
}

mlVariable mlVariable::operator ()(std::vector<mlVariable> args, mlInterpreter* context)
{
	std::vector <mlVariable> result;
	mlVariable m;
	
	if (context == NULL) context = owner;
	
	std::cout << "Argument size: " << args.size() << std::endl;
	std::cout << "Call interpreter variable (" << this->type << "," << subtype << ")" << std::endl;
	std::cout << *this << std::endl;
	switch (this->type)
	{
		case FUNCTION:
			switch(subtype)
			{
				case ST_CFUNCTION:
					return mlCallbackMgr::call(*this, args, context);
				default:
					break;
			}
		case STRING:
			if (context == NULL)
			{
			 	std::cerr << "Error: cannot eval string without owner";
				return result;
			}
			
			if (subtype == ST_CSTRING)
				context->eval(get<char*>());
			else
				context->eval(get<std::string>());
			return result;
	}
}

mlVariable& mlVariable::operator [](std::string name)
{
	this->type = mlVariable::OBJECT;
	this->subtype = mlVariable::ST_NONE;
	if (fields.count(name) == 0)
	{
		fields[name] = new mlVariable();
	}
	return *(fields[name]);
}

mlVariable& mlVariable::operator =(const mlVariable& v)
{
	to_nil();
	data = malloc(v.data_size);
	memcpy(data,v.data,v.data_size);
	data_size = v.data_size;
	type = v.type;
	subtype = v.subtype;
	fields = v.fields;
	type_info = v.type_info;
	return *this;
}


std::ostream& operator <<(std::ostream& str,mlVariable v)
{
	switch (v.type)
	{
		case mlVariable::STRING:
			if (v.subtype == mlVariable::ST_NONE)
				str << "'" << v.get<std::string>() << "' , type: STRING";
			else 
				str << "'" << (v.get<char*>()) << "' type: CSTRING";
			break;
		case mlVariable::NUMBER:
			str << v.get<int>() << " type: NUMBER";
			break;
		case mlVariable::BOOLEAN:
			str << v.get<bool>() << " type: BOOLEAN";
			break;
		case mlVariable::FUNCTION:
			str << " type: FUNCTION ";
			break;
		case mlVariable::OBJECT:
			str << " type: OBJECT.";
			if (v.fields.size() > 0)
			{
				str<< " Field set: {\n";
				for (std::map<std::string, mlVariable*>::iterator i = v.fields.begin() ; i != v.fields.end() ; i++)
				{
					std::string fieldName = i->first;
					mlVariable* value = i->second;
					str << "\tField '" << fieldName << "' : \t";
					str << *value << std::endl;
				}
				str << "}\n";
			}
			break;
		case mlVariable::NILL:
			str << "NILL";
			break;
	}
	int status;
	if (v.type_info != NULL)
		str << " Typeid: " << abi::__cxa_demangle(v.type_info->name(), 0, 0, &status) << " , ";
	str << " Subtype : ";
	switch (v.subtype)
	{
		case mlVariable::ST_NONE:
			str << "none ";
			break;
		case mlVariable::ST_BINARY:
			str << "BINARY";
			break;
		case mlVariable::ST_CFUNCTION:
			str << "CFUNCTION" << "()" << (void*)(v.data);
			break;
		case mlVariable::ST_CSTRING:
			str << "C string";
			break;
	}
	str  << " , Data size: " << v.size();
	return str;
}

std::vector<std::string> mlVariable::keys()
{
	std::vector<std::string> retVal;
	for (std::map<std::string, mlVariable*>::iterator i = fields.begin() ; i != fields.end() ; i++)
		retVal.push_back(i->first);
	return retVal;
}

mlVariable::~mlVariable()
{
	std::vector<std::string> my_keys = keys();
	for (int i=0;i<my_keys.size();i++)
	{
//			delete fields[my_keys[i]];//FIXME
	}
	if (data_size && data)
	{
		free_mem();
		data_size = 0;
	}
}

bool mlVariable::operator ==(const mlVariable& comp)
{ 
	if (comp.type != type)
		return (data_size == comp.data_size && (memcmp(data,comp.data,data_size) == 0) && fields ==comp.fields); 
	switch (get_type())
	{
		case mlVariable::STRING:
			std::string first;
			if ( ((mlVariable&)comp).get_subtype() == mlVariable::ST_CSTRING)
				first = ((mlVariable&)comp).get<char*>();
			else
				first = ((mlVariable&)comp).get<std::string>();
			
			std::string second;
			if ( get_subtype() == mlVariable::ST_CSTRING)
				second =get<char*>();
			else
				second = get<std::string>();
			break;
			return (first == second);
	}
}
