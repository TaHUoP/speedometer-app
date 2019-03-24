#include <Windows.h>
#include <atomic>
#include <stdio.h>
#include <iostream>
#include <chrono>
#include <conio.h>
#include <thread>
#include <numeric> 
 
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
	int avg_speed;
	int normal_speed;
	//speed now is normalized by 100 at first and by max speed if it becomes more than 100
	//TODO: make max_speed decrease with time if current speed is significantly lower
	int max_speed = 0;

	using namespace std::chrono_literals;
	
	constexpr auto sleepTime = 200ms;
	constexpr int typing_coeficient = 60s/sleepTime;
	
	auto now = std::chrono::system_clock::now();
	
	constexpr std::size_t smoothingNum = 20;
	std::array<unsigned int, smoothingNum> SpeedBuf = {0};
	std::size_t ringIdx = 0;

    //TODO: detect com port automatically (requires hardware changes)
    std::cout << "Enter COM number: ";
    int com_num = 0;
    std::cin >> com_num;

	if ((comport = fopen(("COM" + std::to_string(com_num)).c_str(), "wt")) == NULL)
	{
	    std::cout << "Failed to open the communication port COM" << com_num << std::endl;
	    std::cout << "The port may be disabled or in use" << std::endl;
		int wait=getch();
		return 1;
	}
	
	//TODO: set condition to escape the loop
	while (true)
	{
		std::this_thread::sleep_until(now+sleepTime);
		now = std::chrono::system_clock::now();
		
		SpeedBuf[ringIdx] = keypress_counter.exchange(0)*typing_coeficient;
		ringIdx = (ringIdx + 1) % smoothingNum;
		
		avg_speed = std::accumulate(SpeedBuf.cbegin(), SpeedBuf.cend(), 0)/smoothingNum;
		if (avg_speed > max_speed) {
		    max_speed = avg_speed;
		}

		normal_speed = ((avg_speed * 100)/ max_speed);

		printf("\r%d              ", normal_speed);
		
		fflush(comport);
		fputc(normal_speed, comport);
	}
		
	t1.join();
	UnhookWindowsHookEx(_hook);
	fclose(comport);
	return 0;
}