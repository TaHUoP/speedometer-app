#include <Windows.h>
#include <atomic>
#include <stdio.h>
#include <iostream>
#include <chrono>
#include <conio.h>
#include <thread>
 
HHOOK _hook;

KBDLLHOOKSTRUCT kbdStruct;

std::atomic<unsigned int> keypress_counter;

LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0 && wParam == WM_KEYDOWN)
	{
		keypress_counter++;	
	}
 
	return CallNextHookEx(_hook, nCode, wParam, lParam);
}

void catch_keypress()
{
	_hook = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallback, NULL, 0);
	
	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0))
	{
		
	}
}
 
int main()
{	
	std::thread t1(catch_keypress);
		
	FILE *comport;
	int speed;
	
	using namespace std::chrono_literals;
	
	constexpr auto sleepTime = 200ms;
	constexpr int typing_coeficient = 60s/sleepTime;
	
	auto now = std::chrono::system_clock::now();
	
	if ((comport = fopen("COM3", "wt")) == NULL)
	{
		printf("Failed to open the communication port COM3\n");
		printf("The port may be disabled or in use\n");
		int wait=getch();
		return 1;
	}
	
	//TODO: set condition to escape the loop
	while (true)
	{
		std::this_thread::sleep_until(now+sleepTime);
		now = std::chrono::system_clock::now();
		
		speed = keypress_counter.exchange(0)*typing_coeficient;
		
		printf("\r%d              ", speed);
		
		fflush(comport);
		fputc(speed, comport);
	}
		
	t1.join();
	UnhookWindowsHookEx(_hook);
	fclose(comport);
	return 0;
}