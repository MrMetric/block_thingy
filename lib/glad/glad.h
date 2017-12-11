#ifdef __GNUC__
	#pragma GCC system_header
#endif

#ifdef DEBUG_BUILD
	#include "glad.debug.h"
#else
	#include "glad.release.h"
#endif
