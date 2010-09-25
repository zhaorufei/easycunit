#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "ANSI_C_UnitTest.h"

UNIT_TEST(my_test_1)
{
#ifdef WIN
	MessageBox(0, "unit test sample", __FUNCSIG__, MB_OK);
#else
   printf( "calling [%s]\n", __FUNCSIG__);
#endif
}

UNIT_TEST(my_test_2)
{
#ifdef WIN
	MessageBox(0, "unit test sample", __FUNCSIG__, MB_OK);
#else
   printf( "calling [%s]\n", __FUNCSIG__);
#endif
}

#ifdef WIN
#pragma comment(lib, "user32.lib")
static int console_window_shared_unit_test_main(BOOL console_app, int argc, char *argv[])
{
	g_unit_test_cases.reg_done_ = 1;

	// puts(__FUNCSIG__);
	if( argc == 1 ) {
		while(can_exit_flag == 0) {
			// printf("Sleep 1 seconds to wait for parent process\n");
			Sleep(1000);
		}
	} else {
		// Run specified tests
		int i = 0;
		for(i = 1; i < argc; ++i) {
			int k = 0;
			for(k = 0; k < g_unit_test_cases.num_of_cases_; ++k)
			{
				if( strcmp(argv[i],  g_unit_test_cases.test_cases_[k].name_) == 0) {
					if( console_app ) {
						printf("Found test cases [%s] to run...", argv[i]);
						fflush(stdout);
					}
					g_unit_test_cases.test_cases_[k].fp_();
					console_app && printf("Done\n");
				}
			}
		}
	}
	return 0;
}
int WINAPI WinMain(
		HINSTANCE hInstance,
		HINSTANCE hPrevInstance,
		LPTSTR lpCmdLine,
		int nShowCmd)
{
#if 0
	MessageBox(0, "unit test sample", __FUNCSIG__, MB_OK);
	printf("Hello");
	fflush(stdout);
	Sleep(1000);
	printf("World\n");
#else
		// puts(__FUNCSIG__);
	LPWSTR *szArglist = NULL;
	int nArgs         = 0;
	int i             = 0;
	char ** argv      = NULL;
	puts(__FUNCSIG__ );
	MessageBox(0, "unit test sample", __FUNCSIG__, MB_OK);

	szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if( NULL == szArglist )
	{
		printf("CommandLineToArgvW failed\n");
		return 0;
	}

	argv = malloc( sizeof(char*) * nArgs);
	for( i = 0; i<nArgs; i++)
	{
		argv[i] = (char*) malloc( wcslen(szArglist[i]) + 1);
		sprintf(argv[i], "%S", szArglist[i]);
	}
	console_window_shared_unit_test_main(FALSE, nArgs, argv);
	for( i = 0; i < nArgs; ++i) {
		free(argv[i]);
	}
	free(argv);

	LocalFree(szArglist);

	return 0;
#endif
}

#else
int main(int argc,const char*const* argv)
{
	printf("Press <Enter> to continue...");
	fflush(stdout);
	getchar();
	printf("[%s]\n", __FUNCSIG__);
	return 0;
}
#endif
