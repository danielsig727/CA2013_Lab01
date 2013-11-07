#ifndef __UTILS_HH_OGCTK60Y__
#define __UTILS_HH_OGCTK60Y__

#include <CL/cl.h>

void initGenerator( unsigned seed = 1234 );
void genData( int* &data );
void deleteData( int* &data );
void buildWithBinary( cl_program &mProgram, cl_context &mContext, const cl_device_id* const mDevice );

#endif /* end of include guard: UTILS_HH_OGCTK60Y */

