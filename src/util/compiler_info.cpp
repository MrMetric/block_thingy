#include "compiler_info.hpp"

using std::string;

namespace block_thingy::util {

#if !defined(__ICC) && defined(__ICL)
	// TODO: test this
	#define __ICC __ICL
#endif

#if defined(__ICC)
static const char info[]
{
	// C2 AE is UTF-8 for ®
	'I', 'n', 't', 'e', 'l', 0xC2, 0xAE, ' ',
	'C', '+', '+', ' ',
	'C', 'o', 'm', 'p', 'i', 'l', 'e', 'r', ' ',
	(__ICC / 1000) % 10 + '0',
	(__ICC /  100) % 10 + '0',
	'.',
	(__ICC /   10) % 10 + '0',
	'.',
	(__ICC /    1) % 10 + '0',
	' ',
	(__INTEL_COMPILER_BUILD_DATE / 10000000) % 10 + '0',
	(__INTEL_COMPILER_BUILD_DATE /  1000000) % 10 + '0',
	(__INTEL_COMPILER_BUILD_DATE /   100000) % 10 + '0',
	(__INTEL_COMPILER_BUILD_DATE /    10000) % 10 + '0',
	(__INTEL_COMPILER_BUILD_DATE /     1000) % 10 + '0',
	(__INTEL_COMPILER_BUILD_DATE /      100) % 10 + '0',
	(__INTEL_COMPILER_BUILD_DATE /       10) % 10 + '0',
	(__INTEL_COMPILER_BUILD_DATE /        1) % 10 + '0',
};
#elif defined(_MSC_VER)
static const char info[]
{
	'M', 'S', 'V', 'C', ' ',
	(_MSC_FULL_VER / 100000000) % 10 + '0',
	(_MSC_FULL_VER /  10000000) % 10 + '0',
	'.',
	(_MSC_FULL_VER /   1000000) % 10 + '0',
	(_MSC_FULL_VER /    100000) % 10 + '0',
	'.',
	(_MSC_FULL_VER /     10000) % 10 + '0',
	(_MSC_FULL_VER /      1000) % 10 + '0',
	(_MSC_FULL_VER /       100) % 10 + '0',
	(_MSC_FULL_VER /        10) % 10 + '0',
	(_MSC_FULL_VER /         1) % 10 + '0',
	'.',
	(_MSC_BUILD / 10) % 10 + '0',
	(_MSC_BUILD /  1) % 10 + '0',
};
#endif

string compiler_info()
{
#if defined(__clang__)
	return "Clang " __clang_version__;
#elif defined(__INTEL_COMPILER)
	return info;
#elif defined(__GNUC__)
	return "GCC " __VERSION__;
#elif defined(_MSC_VER)
	return info;
#else
	return "(unknown compiler)";
#endif
}

}
