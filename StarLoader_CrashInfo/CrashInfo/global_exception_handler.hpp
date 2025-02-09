/**
 * Copyright - xWhitey, 2025.
 * global_exception_handler.hpp - description
 *
 * CrashInfo (unhandled exception handler) header file
 * Authors: xWhitey. Refer to common.hpp file for dependencies and their authors.
 * Do not delete this comment block. Respect others' work!
 */

#ifdef GLOBAL_EXCEPTION_HANDLER_HPP_RECURSE_GUARD
#error Recursive header files inclusion detected in global_exception_handler.hpp
#else //GLOBAL_EXCEPTION_HANDLER_HPP_RECURSE_GUARD

#define GLOBAL_EXCEPTION_HANDLER_HPP_RECURSE_GUARD

#ifndef GLOBAL_EXCEPTION_HANDLER_HPP_GUARD
#define GLOBAL_EXCEPTION_HANDLER_HPP_GUARD
#pragma once

#ifdef __cplusplus

typedef struct CGlobalExceptionHandler {
	static void Initialise();
	static void Shutdown();
	static LRESULT CALLBACK CBTHookProc(INT _Code, WPARAM _WordParam, LPARAM _LongParam);
	static LONG CALLBACK RunExceptionHandler(PEXCEPTION_POINTERS _Pointers);

	static HHOOK ms_hHook;
	static CRITICAL_SECTION ms_CriticalSection;
	static void* ms_pExceptionHandlerHandle;

	typedef BOOL (WINAPI* _PlaySoundA)(LPCTSTR pszSound, HMODULE hmod, DWORD fdwSound);
	static _PlaySoundA ms_pfnPlaySoundA;
} CGlobalExceptionHandler;

using CGlobalExceptionHandler = struct CGlobalExceptionHandler;

#else //!__cplusplus
#error C++ compiler required to compile global_exception_handler.hpp
#endif //__cplusplus

#endif //GLOBAL_EXCEPTION_HANDLER_HPP_GUARD

#undef GLOBAL_EXCEPTION_HANDLER_HPP_RECURSE_GUARD
#endif //GLOBAL_EXCEPTION_HANDLER_HPP_RECURSE_GUARD