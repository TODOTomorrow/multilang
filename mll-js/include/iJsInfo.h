#ifndef __ILUAINFO_H__
#define __ILUAINFO_H__
#include <mll/mlInterpreterInfo.h>
class iJsInfo : public mlInterpreterInfo
{
	public:
		std::string get_name () { return "Javascript"; }
};
extern "C"
{
	iJsInfo* mllib_get_info();
}
#endif
