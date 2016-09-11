#ifndef __MLSUPPORT_H__
#define __MLSUPPORT_H__
#include <mll/mlInterpreter.h>

class A
{
	public:
		void test();
};

extern "C"
{
	void init_introspector(mlInterpreter* in);
}
#endif
