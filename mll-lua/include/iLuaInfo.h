#ifndef __ILUAINFO_H__
#define __ILUAINFO_H__
#include <mll/mlInterpreterInfo.h>
class iLuaInfo : public mlInterpreterInfo
{
	public:
		std::string get_name () { return "Lua"; }
};
extern "C"
{
	iLuaInfo* mllib_get_info();
}
#endif
