#include <tchar.h>
#include <windows.h>
#include <stdio.h>

// Command line to compile:
// cl /MDd /Zi test.c
// cl /Zi test.c

#ifdef WIN
#pragma comment(lib, "user32.lib")
#pragma comment(linker, "/subsystem:windows")
#pragma comment(linker, "/export:unit_test_main=_unit_test_main@16")
int WINAPI unit_test_main(
		HINSTANCE hInstance,
		HINSTANCE hPrevInstance,
		LPTSTR lpCmdLine,
		int nShowCmd)
{
	// puts(__FUNCSIG__);
	MessageBox(0, "ANSI C auto unit test", __FUNCSIG__, MB_OK);
	return 0;
}

int WINAPI _tWinMain(
		    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPTSTR lpCmdLine,
    int nShowCmd)
{
	// getchar();
	MessageBox(0, "ANSI C auto unit test", __FUNCSIG__, MB_OK);
	printf("my_function: %p\n", unit_test_main);
	return 0;
}

#else
__declspec(dllexport) int number_of_au;
__declspec(dllexport) int can_exit_flag;

__declspec(dllexport) int unit_test_main()
{
	puts(__FUNCSIG__);
	number_of_au = 100;
	while(can_exit_flag == 0) {
		printf("Sleep 1 seconds to wait for parent process\n");
		Sleep(1000);
	}
	return 0;
}

int main()
{
	printf("Press <Enter> to continue...");
	fflush(stdout);
	getchar();
	// printf("unit_test_main: %p\n", unit_test_main);
	puts(__FUNCSIG__);
	return 0;
}
#endif
