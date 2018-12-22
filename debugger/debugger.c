#include "pch.h"

int handleDebugEvent(const DEBUG_EVENT dv, const HANDLE proc, DWORD* status);

HANDLE startDebuggee(const LPCWSTR procName)
{
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcess(procName, NULL, NULL, NULL, 0, DEBUG_ONLY_THIS_PROCESS | PROCESS_VM_READ, NULL, NULL, &si, &pi))
		return 0;

	return pi.hProcess;
}

int main(int argc, const char** argv)
{
	wchar_t PROC_NAME[] = TEXT("C:/git/de-anti-debugging/x64/Debug/debuggee.exe");

	HANDLE proc = startDebuggee(PROC_NAME);
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

int handleDebugEvent(const DEBUG_EVENT dv, const HANDLE proc, DWORD* status)
{
	wchar_t* wmsg;
	char* msg;
	OUTPUT_DEBUG_STRING_INFO messageInfo;
	EXCEPTION_DEBUG_INFO exception;

	switch (dv.dwDebugEventCode)
	{
	case OUTPUT_DEBUG_STRING_EVENT:
		messageInfo = dv.u.DebugString;

		printf(">>> Debug output string\n");
		if (messageInfo.fUnicode)
		{
			wmsg = malloc(messageInfo.nDebugStringLength);
			if (ReadProcessMemory(proc, messageInfo.lpDebugStringData, wmsg, messageInfo.nDebugStringLength, NULL))
			{
				printf("%s\n", wmsg);
			}
			else
			{
				printf("Could not read from memory!\n");
			}

			free(wmsg);
		}
		else
		{
			msg = malloc(messageInfo.nDebugStringLength);
			if (ReadProcessMemory(proc, messageInfo.lpDebugStringData, msg, messageInfo.nDebugStringLength, NULL))
			{
				printf("%s\n", msg);
			}
			else
			{
				printf("Could not read from memory!\n");
			}
			free(msg);
		}

		return 1;
	case CREATE_PROCESS_DEBUG_EVENT:
		printf(">>> Process was created!!\n");
		return 1;
	case LOAD_DLL_DEBUG_EVENT:
		printf(">>> A DLL was loaded!!\n");
		return 1;
	case CREATE_THREAD_DEBUG_EVENT:
		printf(">>> Thread 0x%x (Id: %d) created at: 0x%x\n",
			dv.u.CreateThread.hThread,
			dv.dwThreadId,
			dv.u.CreateThread.lpStartAddress);
		return 1;
	case EXIT_THREAD_DEBUG_EVENT:
		printf(">>> The thread %d exited with code: %d\n",
			dv.dwThreadId,
			dv.u.ExitThread.dwExitCode);
		return 1;
	case UNLOAD_DLL_DEBUG_EVENT:
		printf(">>> A DLL was unloaded!!\n");
		return 1;
	case EXIT_PROCESS_DEBUG_EVENT:
		printf(">>> Process exited with code:  0x%x\n",
			dv.u.ExitProcess.dwExitCode);
		return 0;
	case EXCEPTION_DEBUG_EVENT:
		exception = dv.u.Exception;
		switch (exception.ExceptionRecord.ExceptionCode)
		{
		case EXCEPTION_BREAKPOINT:
			printf(">>> Breakpoint encountered!!!\n");
			return 1;

		default:
			status = DBG_EXCEPTION_NOT_HANDLED;
			if (exception.dwFirstChance == 1)
			{
				printf(">>> First chance exception at %x, exception-code: 0x%08x\n",
					exception.ExceptionRecord.ExceptionAddress,
					exception.ExceptionRecord.ExceptionCode);
			}
			return 1;
		}
	default:
		printf(">>> unknown debug event!\n");
		return 0;
	}
}