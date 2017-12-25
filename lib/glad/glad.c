#ifdef __GNUC__
	#pragma GCC diagnostic ignored "-Wpedantic"
#endif

#ifdef BT_DEBUG_BUILD
	#include "glad.debug.c"
#else
	#include "glad.release.c"
#endif
