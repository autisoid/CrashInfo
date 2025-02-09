/**
 * Copyright - xWhitey, 2025.
 * global_exception_handler.cpp - description
 *
 * CrashInfo (unhandled exception handler) source file
 * Authors: xWhitey. Refer to common.hpp file for dependencies and their authors.
 * Do not delete this comment block. Respect others' work!
 */

#include "StdAfx.h"
#include "global_exception_handler.hpp"
#include "resource.h"
#include <TlHelp32.h>

HHOOK CGlobalExceptionHandler::ms_hHook = nullptr;
CRITICAL_SECTION CGlobalExceptionHandler::ms_CriticalSection;
void* CGlobalExceptionHandler::ms_pExceptionHandlerHandle = nullptr;
CGlobalExceptionHandler::_PlaySoundA CGlobalExceptionHandler::ms_pfnPlaySoundA = nullptr;

const char* GetExceptionName(_In_ unsigned long _ExceptionCode) {
	switch (_ExceptionCode) {
	case EXCEPTION_ACCESS_VIOLATION:
		return "EXCEPTION_ACCESS_VIOLATION";
	case EXCEPTION_DATATYPE_MISALIGNMENT:
		return "EXCEPTION_DATATYPE_MISALIGNMENT";
	case EXCEPTION_BREAKPOINT:
		return "EXCEPTION_BREAKPOINT";
	case EXCEPTION_SINGLE_STEP:
		return "EXCEPTION_SINGLE_STEP";
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
	case EXCEPTION_FLT_DENORMAL_OPERAND:
		return "EXCEPTION_FLT_DENORMAL_OPERAND";
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
	case EXCEPTION_FLT_INEXACT_RESULT:
		return "EXCEPTION_FLT_INEXACT_RESULT";
	case EXCEPTION_FLT_INVALID_OPERATION:
		return "EXCEPTION_FLT_INVALID_OPERATION";
	case EXCEPTION_FLT_OVERFLOW:
		return "EXCEPTION_FLT_OVERFLOW";
	case EXCEPTION_FLT_STACK_CHECK:
		return "EXCEPTION_FLT_STACK_CHECK";
	case EXCEPTION_FLT_UNDERFLOW:
		return "EXCEPTION_FLT_UNDERFLOW";
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
		return "EXCEPTION_INT_DIVIDE_BY_ZERO";
	case EXCEPTION_INT_OVERFLOW:
		return "EXCEPTION_INT_OVERFLOW";
	case EXCEPTION_PRIV_INSTRUCTION:
		return "EXCEPTION_PRIV_INSTRUCTION";
	case EXCEPTION_IN_PAGE_ERROR:
		return "EXCEPTION_IN_PAGE_ERROR";
	case EXCEPTION_ILLEGAL_INSTRUCTION:
		return "EXCEPTION_ILLEGAL_INSTRUCTION";
	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
		return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
	case EXCEPTION_STACK_OVERFLOW:
		return "EXCEPTION_STACK_OVERFLOW";
	case EXCEPTION_INVALID_DISPOSITION:
		return "EXCEPTION_INVALID_DISPOSITION";
	case EXCEPTION_GUARD_PAGE:
		return "EXCEPTION_GUARD_PAGE";
	case EXCEPTION_INVALID_HANDLE:
		return "EXCEPTION_INVALID_HANDLE";
	default:
		return nullptr;
	}

	return nullptr;
}

void CGlobalExceptionHandler::Initialise() {
	InitializeCriticalSection(&ms_CriticalSection);

	ms_pExceptionHandlerHandle = AddVectoredExceptionHandler(1, &RunExceptionHandler);

	HMODULE hLibrary = LoadLibraryA("winmm");
	if (hLibrary) {
		ms_pfnPlaySoundA = (_PlaySoundA)GetProcAddress(hLibrary, "PlaySoundA");
	}
}

void CGlobalExceptionHandler::Shutdown() {
	RemoveVectoredExceptionHandler(ms_pExceptionHandlerHandle);

	DeleteCriticalSection(&ms_CriticalSection);

	if (ms_hHook)
		UnhookWindowsHookEx(ms_hHook);
}

LRESULT CALLBACK CGlobalExceptionHandler::CBTHookProc(INT _Code, WPARAM _WordParam, LPARAM _LongParam) {
	if (_Code == HCBT_CREATEWND) {
		HWND hMsgBox = (HWND)_WordParam;
		LONG_PTR style = GetWindowLongPtr(hMsgBox, GWL_STYLE);
		SetWindowLongPtr(hMsgBox, GWL_STYLE, style & ~WS_SYSMENU);
	}

	return CallNextHookEx(ms_hHook, _Code, _WordParam, _LongParam);
}

LONG CALLBACK CGlobalExceptionHandler::RunExceptionHandler(PEXCEPTION_POINTERS _Pointers) {
	DWORD dwExceptionCode = _Pointers->ExceptionRecord->ExceptionCode;

	if (dwExceptionCode == 0xE06D7363u) // SEH
		return EXCEPTION_CONTINUE_SEARCH;

	_fpreset();
	EnterCriticalSection(&ms_CriticalSection);

	char rgchPrintBuffer[32768];
	char rgchTempBuffer[32768];
	memset(rgchPrintBuffer, 0, sizeof(rgchPrintBuffer));
	memset(rgchTempBuffer, 0, sizeof(rgchTempBuffer));
	_snprintf_s(rgchPrintBuffer, sizeof(rgchPrintBuffer), "Game has crashed, here's a crash log:\n");
	DWORD dwProcessId = GetCurrentProcessId();
	DWORD dwThreadId = GetCurrentThreadId();

	_snprintf_s(rgchTempBuffer, sizeof(rgchTempBuffer), "Current process ID: %lu\n", dwProcessId);
	strncat_s(rgchPrintBuffer, rgchTempBuffer, sizeof(rgchPrintBuffer));
	_snprintf_s(rgchTempBuffer, sizeof(rgchTempBuffer), "Current thread ID: %lu\n", dwThreadId);
	strncat_s(rgchPrintBuffer, rgchTempBuffer, sizeof(rgchPrintBuffer));

	DWORD dwInstructionPointer = _Pointers->ContextRecord->Eip;

	const char* pszExceptionCodeName = GetExceptionName(dwExceptionCode);
	if (!pszExceptionCodeName) {
		pszExceptionCodeName = "UNKNOWN";
		LeaveCriticalSection(&ms_CriticalSection);

		return EXCEPTION_EXECUTE_HANDLER; //We ain't handling that exception, no.
	}

	PVOID pExceptionAddress = _Pointers->ExceptionRecord->ExceptionAddress;
	char rgchExceptionModuleAddress[MAX_PATH] = "module not found";
	char rgchInstructionPointerAddress[MAX_PATH] = "module not found";

	do {
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, NULL);

		if (hSnapshot != INVALID_HANDLE_VALUE) {
			MODULEENTRY32 entry;
			entry.dwSize = sizeof(entry);

			bool bHasExceptionModuleBeenFound = false;
			bool bHasInstructionPointerModuleBeenFound = false;

			bool bIsModuleInfoRead = Module32First(hSnapshot, &entry) != 0;

			while (bIsModuleInfoRead) {
				if (!bHasExceptionModuleBeenFound && pExceptionAddress >= entry.modBaseAddr && pExceptionAddress < entry.modBaseAddr + entry.modBaseSize) {
					bHasExceptionModuleBeenFound = true;

					_snprintf_s(rgchExceptionModuleAddress, sizeof(rgchExceptionModuleAddress), "\"%s\"+0x%x", entry.szModule, ((unsigned char*)pExceptionAddress - entry.modBaseAddr));

					if (bHasExceptionModuleBeenFound && bHasInstructionPointerModuleBeenFound)
						break;
				}

				if (!bHasInstructionPointerModuleBeenFound && (unsigned char*)dwInstructionPointer >= entry.modBaseAddr && (unsigned char*)dwInstructionPointer < entry.modBaseAddr + entry.modBaseSize) {
					bHasInstructionPointerModuleBeenFound = true;

					_snprintf_s(rgchInstructionPointerAddress, sizeof(rgchInstructionPointerAddress), "\"%s\"+0x%x", entry.szModule, ((unsigned char*)dwInstructionPointer - entry.modBaseAddr));

					if (bHasExceptionModuleBeenFound && bHasInstructionPointerModuleBeenFound)
						break;
				}

				bIsModuleInfoRead = Module32Next(hSnapshot, &entry) != 0;
			}

			CloseHandle(hSnapshot);
		} else {
			strncpy_s(rgchExceptionModuleAddress, sizeof(rgchExceptionModuleAddress), "Error: snapshot = INVALID_HANDLE_VALUE", sizeof(rgchExceptionModuleAddress) - 1);
			strncpy_s(rgchInstructionPointerAddress, sizeof(rgchInstructionPointerAddress), rgchExceptionModuleAddress, sizeof(rgchInstructionPointerAddress) - 1);
		}
	} while (0);

	_snprintf_s(rgchTempBuffer, sizeof(rgchTempBuffer), "Exception address: 0x%p (%s)\n", pExceptionAddress, rgchExceptionModuleAddress);
	strncat_s(rgchPrintBuffer, rgchTempBuffer, sizeof(rgchPrintBuffer));

	_snprintf_s(rgchTempBuffer, sizeof(rgchTempBuffer), "Exception code: 0x%X (%s)\n", dwExceptionCode, pszExceptionCodeName);
	strncat_s(rgchPrintBuffer, rgchTempBuffer, sizeof(rgchPrintBuffer));

	_snprintf_s(rgchTempBuffer, sizeof(rgchTempBuffer),
		"\nGeneral registers:"
		"\nEAX: 0x%08X (%d)"
		"\nEBX: 0x%08X (%d)"
		"\nECX: 0x%08X (%d)"
		"\nEDX: 0x%08X (%d)"
		"\nESI: 0x%08X (%d)"
		"\nEDI: 0x%08X (%d)"
		"\nEBP: 0x%08X (%d)"
		"\nESP: 0x%08X (%d)"
		"\nEIP: 0x%08X (%d) [%s]"
		"\nEFL: 0x%08X (%d)\n",

		_Pointers->ContextRecord->Eax, _Pointers->ContextRecord->Eax,
		_Pointers->ContextRecord->Ebx, _Pointers->ContextRecord->Ebx,
		_Pointers->ContextRecord->Ecx, _Pointers->ContextRecord->Ecx,
		_Pointers->ContextRecord->Edx, _Pointers->ContextRecord->Edx,
		_Pointers->ContextRecord->Esi, _Pointers->ContextRecord->Esi,
		_Pointers->ContextRecord->Edi, _Pointers->ContextRecord->Edi,
		_Pointers->ContextRecord->Ebp, _Pointers->ContextRecord->Ebp,
		_Pointers->ContextRecord->Esp, _Pointers->ContextRecord->Esp,
		_Pointers->ContextRecord->Eip, _Pointers->ContextRecord->Eip, rgchInstructionPointerAddress,
		_Pointers->ContextRecord->EFlags, _Pointers->ContextRecord->EFlags
	);
	strncat_s(rgchPrintBuffer, rgchTempBuffer, sizeof(rgchPrintBuffer));
	_snprintf_s(rgchTempBuffer, sizeof(rgchTempBuffer),
		"\nSegment registers:"
		"\nCS: %04X"
		"\nDS: %04X"
		"\nES: %04X"
		"\nFS: %04X"
		"\nGS: %04X"
		"\nSS: %04X",

		_Pointers->ContextRecord->SegCs,
		_Pointers->ContextRecord->SegDs,
		_Pointers->ContextRecord->SegEs,
		_Pointers->ContextRecord->SegFs,
		_Pointers->ContextRecord->SegGs,
		_Pointers->ContextRecord->SegSs
	);
	strncat_s(rgchPrintBuffer, rgchTempBuffer, sizeof(rgchPrintBuffer));

	if (ms_pfnPlaySoundA)
		ms_pfnPlaySoundA(MAKEINTRESOURCEA(EXCEPTION_HANDLER_MUSIC), (HMODULE) g_pSelfHandle, SND_RESOURCE | SND_ASYNC | SND_LOOP);

	MessageBoxA(NULL, rgchPrintBuffer, PLUGIN_NAME, NULL);

	ms_hHook = SetWindowsHookEx(WH_CBT, CBTHookProc, NULL, GetCurrentThreadId());
	LeaveCriticalSection(&ms_CriticalSection);
	UnhookWindowsHookEx(ms_hHook);

	return EXCEPTION_CONTINUE_SEARCH;
}