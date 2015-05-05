# The simple\_vc\_gcc.c file: contains 2 unit test cases #
```
#ifdef _MSC_VER
#    include <windows.h>
#endif
#include <stdio.h>
#include <string.h>
#include "ANSI_C_UnitTest.h"

#if defined(_MSC_VER ) && defined(WIN)
#   pragma comment(lib, "user32.lib")
#   define  I_AM_CALLED  MessageBox(0, "unit test sample", __FUNCSIG__, MB_OK);
#else
#   define  I_AM_CALLED  printf( "calling [%s]\n", __FUNCSIG__);
#endif

UNIT_TEST(my_test_1)
{
	I_AM_CALLED;
}

UNIT_TEST(my_test_2)
{
	I_AM_CALLED;
}

#if defined(WIN) && defined(_MSC_VER)
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nShowCmd)
{
	MessageBox(0, "unit test sample", __FUNCSIG__, MB_OK);
	return 0;
}

#else
int main(int argc,const char*const* argv)
{
	printf("calling [%s]\n", __FUNCSIG__);
	return 0;
}
#endif
```

# Running by itself and by CURunner #
vcvars32.bat
cl /Zi simple\_vc\_gcc.c
## Run normally ##
simple\_vc\_gcc.exe

Output:
```
calling [int __cdecl main(int,const char *const *)]
```

## Run by CURunner.exe ##
CURunner.exe simple\_vc\_gcc.exe

Output:
```
command line: [simple_vc_gcc.exe]
tmainCRTStartup: 004015B2
replace call main/WinMain: 0040170D with 004013B0
Resuming process [simple_vc_gcc.exe].
004230D8, num: 2
unit test case: [my_test_1]
unit test case: [my_test_2]
```

Note that the main function is not called and CURunner detected 2 unit test cases, as listed.

## Run specified uni test by CURunner.exe ##
CURunner.exe /t:my\_test\_1 simple\_vc\_gcc.exe

```
command line: [simple_vc_gcc.exe my_test_1]
tmainCRTStartup: 0040196F
replace call main/WinMain: 00401ACA with 004013E0
Resuming process [simple_vc_gcc.exe].

T:\zrf\PracticalCpp\easycunit>Found test cases [my_test_1] to run...calling [void __cdecl my_test_1(void)]
Done
```

Note that the program is not optmized for output.