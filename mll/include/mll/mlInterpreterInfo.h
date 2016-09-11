#ifndef __MLINTERPRETERINFO_H__
#define __MLINTERPRETERINFO_H__
#include <string>
#include <ostream>

class mlInterpreterInfo
{
	public:
		enum capabilities_t 
		{
			file_exec_no_sup = (1 << 0)
		};
		static const capabilities_t capab_all = (capabilities_t)0;
		virtual std::string get_name () {}
		virtual capabilities_t get_capabilities() {return mlInterpreterInfo::capab_all;}
		friend std::ostream& operator<< (std::ostream& stream, mlInterpreterInfo& info)
		{
			stream << "Interpreter information:\n\tName:" << info.get_name() << std::endl;
			return stream;
		}
};
#endif
