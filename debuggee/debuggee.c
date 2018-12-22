#include "pch.h"

const char success[] = "no debugger was found...";
const char failure[] = "I CAN SEE YOU OVER THERE!";

int main()
{
	if (IsDebuggerPresent())
	{
		OutputDebugStringA(failure);
	}
	else
	{
		OutputDebugStringA(success);
	}
}