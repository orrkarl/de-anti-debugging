// helloworld.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>

auto success = "no debugger was found...\n";
auto failure = "I CAN SEE YOU OVER THERE!\n";

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