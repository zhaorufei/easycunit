#ifdef _MSC_VER
#   include <windows.h>
#   include <Shellapi.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include "easy-c-unit.h"

#ifdef _MSC_VER
    #pragma comment(lib, "shell32.lib")
    #pragma comment(lib, "user32.lib")
#endif
struct UnitTestCases g_unit_test_cases = {0, 0, MAX_NUM_OF_TEST_CASES};
int can_exit_flag;

void register_unit_test(const char * name, int len, const char * file, int line, UnitTest_FP fp)
{
	if( g_unit_test_cases.num_of_cases_ >= MAX_NUM_OF_TEST_CASES ) {
		static int ever_reported = 0;
		if( ever_reported == 0) {
			ever_reported = 1;
			fprintf(stderr, "Number of test cases exceeds maximum test cases(%d), test case [%s] ignored\n"
					, MAX_NUM_OF_TEST_CASES, name);
		}
		return;
	}
    g_unit_test_cases.test_cases_[g_unit_test_cases.num_of_cases_].name_len_ = len;
    g_unit_test_cases.test_cases_[g_unit_test_cases.num_of_cases_].name_ = name;
    g_unit_test_cases.test_cases_[g_unit_test_cases.num_of_cases_].file_ = file;
    g_unit_test_cases.test_cases_[g_unit_test_cases.num_of_cases_].line_ = line;
    g_unit_test_cases.test_cases_[g_unit_test_cases.num_of_cases_].fp_ = fp;
    g_unit_test_cases.num_of_cases_ ++;
}

#if _MSC_VER
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

#pragma comment(linker, "/export:unit_test_WinMain=_unit_test_WinMain@16")
int WINAPI unit_test_WinMain(
		HINSTANCE hInstance,
		HINSTANCE hPrevInstance,
		LPTSTR lpCmdLine,
		int nShowCmd)
{
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
}

__declspec(dllexport) int unit_test_main(int argc, char *argv[])
{
	return console_window_shared_unit_test_main(TRUE, argc, argv);
}
#endif
