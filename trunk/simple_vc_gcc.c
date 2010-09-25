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
