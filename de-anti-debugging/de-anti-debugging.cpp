// de-anti-debugging.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

bool handleDebugEvent(const DEBUG_EVENT& dv, const HANDLE proc, DWORD& status);

HANDLE startDebuggee(const LPCWSTR procName)
{
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcess(procName, nullptr, nullptr, nullptr, FALSE, DEBUG_ONLY_THIS_PROCESS | PROCESS_VM_READ, nullptr, nullptr, &si, &pi))
		return 0;

	return pi.hProcess;
}

int main(int argc, const char** argv)
{
	auto PROC_NAME = TEXT("C:/git/de-anti-debugging/Debug/helloworld.exe");


	auto proc = startDebuggee(PROC_NAME);
	DWORD debugStatus = DBG_CONTINUE;
	DEBUG_EVENT dv = { 0 };

	do
	{
		if (!WaitForDebugEvent(&dv, INFINITE))
		{
			break;
		}
		else
		{
			ContinueDebugEvent(dv.dwProcessId, dv.dwThreadId, debugStatus);
		}
	} while (handleDebugEvent(dv, proc, debugStatus));

}

bool handleDebugEvent(const DEBUG_EVENT & dv, const HANDLE proc, DWORD & status)
{
	TCHAR* msg;
	CString eventMessage;

	switch (dv.dwDebugEventCode)
	{
	case OUTPUT_DEBUG_STRING_EVENT:
		OUTPUT_DEBUG_STRING_INFO messageInfo = dv.u.DebugString;


		std::cout << ">>> Debug output string:" << std::endl;
		if (messageInfo.fUnicode)
		{
			auto msg = new WCHAR[messageInfo.nDebugStringLength / 2];
			if (ReadProcessMemory(proc, messageInfo.lpDebugStringData, msg, messageInfo.nDebugStringLength, nullptr))
			{
				std::wcout << msg << std::endl;
			}
			else
			{
				std::cerr << "Could not read from memory!" << std::endl;
			}
			
			delete[] msg;
		}
		else
		{
			auto msg = new char[messageInfo.nDebugStringLength];
			if (ReadProcessMemory(proc, messageInfo.lpDebugStringData, msg, messageInfo.nDebugStringLength, nullptr))
			{
				std::cout << msg << std::endl;
			}
			else
			{
				std::cerr << "Could not read from memory!" << std::endl;
			}
			delete[] msg;
		}

		return true;
	case CREATE_PROCESS_DEBUG_EVENT:
		std::cout << ">>> Process was created!!" << std::endl;
		return true;
	case LOAD_DLL_DEBUG_EVENT:
		std::cout << ">>> A DLL was loaded!!" << std::endl;
		return true;
	case CREATE_THREAD_DEBUG_EVENT:
		eventMessage.Format(L">>> Thread 0x%x (Id: %d) created at: 0x%x",
			dv.u.CreateThread.hThread,
			dv.dwThreadId,
			dv.u.CreateThread.lpStartAddress);
		std::wcout << eventMessage.GetString() << std::endl;
		return true;
	case EXIT_THREAD_DEBUG_EVENT:
		eventMessage.Format(_T(">>> The thread %d exited with code: %d"),
			dv.dwThreadId,
			dv.u.ExitThread.dwExitCode);
		std::wcout << eventMessage.GetString() << std::endl;
		return true;
	case UNLOAD_DLL_DEBUG_EVENT:
		std::cout << ">>> A DLL was unloaded!!" << std::endl;
		return true;
	case EXIT_PROCESS_DEBUG_EVENT:
		eventMessage.Format(L">>> Process exited with code:  0x%x",
			dv.u.ExitProcess.dwExitCode);
		std::wcout << eventMessage.GetString() << std::endl;
		return false;
	case EXCEPTION_DEBUG_EVENT:
		EXCEPTION_DEBUG_INFO exception = dv.u.Exception;
		switch (exception.ExceptionRecord.ExceptionCode)
		{
		case EXCEPTION_BREAKPOINT:
			std::cout << ">>> Breakpoint encountered!!!" << std::endl;
			return true;

		default:
			status = DBG_EXCEPTION_NOT_HANDLED;
			if (exception.dwFirstChance == 1)
			{
				eventMessage.Format(L">>> First chance exception at %x, exception-code: 0x%08x",
					exception.ExceptionRecord.ExceptionAddress,
					exception.ExceptionRecord.ExceptionCode);
				std::wcout << eventMessage.GetString() << std::endl;
			}
			return true;
		}
	default:
		std::cout << ">>> unknown debug event!" << std::endl;
		return false;
	}
}
