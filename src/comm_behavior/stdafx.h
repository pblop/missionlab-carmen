// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#if defined(_WIN32) || defined(WIN32) 
  #pragma once
  #define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
  #define WINDOWS
#endif

#ifdef WINDOWS /* WINDOWS */
 #include <windows.h>                                  
#else                /* other (try linux) */
 #include <unistd.h>
 #include <sys/unistd.h>
#endif

