#ifdef __GNUC__
	#pragma GCC diagnostic ignored "-Wpedantic"
#endif

#ifdef DEBUG_BUILD
	#include "glad.debug.c"
#else
	#include "glad.release.c"
#endif
