#ifndef __INTERLOCK_H__
#define __INTERLOCK_H__

#if defined (__APPLE__)
    #include "interlock_arm_ios.h"
#elif defined (__ANDROID__)
    #include "interlock_gcc.h"
#elif defined(__ARM_MERLIN__)
    #include "interlock_general.h"
#elif defined(__arm__)
    #include "interlock_gcc.h"
#elif defined(__mips__)
 #if __GNUC__ < 4
	#include "interlock_general.h"
 #else
	#include "interlock_gcc.h"
 #endif	
#elif WIN32 
    #include "interlock_x86.h"
#else
    #include "interlock_gcc.h"
#endif


#endif //__INTERLOCK_H__
