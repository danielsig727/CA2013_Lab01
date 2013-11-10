#ifndef __DEFINES_HH_VI0HFZEA__
#define __DEFINES_HH_VI0HFZEA__

#include <cstdlib>
extern size_t DATA_SIZE;

/*Add your new defines below*/
#define MSCHED

#ifndef NV
#define GPU_KERNLIM 2560  // AMD HD7800
#else
#define GPU_KERNLIM 512   // NV 9500 GT
#endif

#endif /* end of include guard: DEFINES_HH_VI0HFZEA */

