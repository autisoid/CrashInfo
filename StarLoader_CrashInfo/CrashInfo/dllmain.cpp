/**
 * Copyright - xWhitey, 2025.
 * dllmain.cpp - The file with our DLL entrypoint.
 *
 * CrashInfo (unhandled exception handler) source file
 * Authors: xWhitey. Refer to common.hpp file for dependencies and their authors.
 * Do not delete this comment block. Respect others' work!
 */

#include "StdAfx.h"
#include "global_exception_handler.hpp"

void* g_pSelfHandle;

//Can also be "ASIMain"
//it MUST be extern "C" because C++ has name mangling so GetProcAddress will fail
extern "C" void __declspec(dllexport) __cdecl InitializeASI() {
	CGlobalExceptionHandler::Initialise();
}

//Can also be DeinitializeASI
//it MUST be extern "C" because C++ has name mangling so GetProcAddress will fail
extern "C" void __declspec(dllexport) __cdecl ASIShutdown() {
	CGlobalExceptionHandler::Shutdown();
}

DWORD WINAPI DllMain( _In_ void* _DllHandle, _In_ unsigned long _Reason, _In_opt_ void* _Reserved )
{
	if (_Reason == DLL_PROCESS_ATTACH)
	{
		g_pSelfHandle = _DllHandle;
	}

	return TRUE;
}