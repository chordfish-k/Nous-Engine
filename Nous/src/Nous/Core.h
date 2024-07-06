#pragma once

#ifdef NS_PLATFORM_WINDOWS
#ifdef NS_BUILD_DLL
		#define NOUS_API __declspec(dllexport)
	#else
		#define NOUS_API __declspec(dllimport)
	#endif // NS_BUILD_DLL

#else
#error Support Windows Only!
#endif // NS_PLATFORM_WINDOWS
