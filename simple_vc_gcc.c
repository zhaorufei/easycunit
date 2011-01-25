// compile: gcc -Wl,-E -o simple_vc_gcc simple_vc_gcc.c -leasy-c-unit
#ifdef _MSC_VER
#    define FUNC_NAME  __FUNCTION__
#    include <windows.h>
#elif defined(__GNUC__)
#    define FUNC_NAME  __func__
#else
#error Only VC++ and GCC are supported at present
#endif
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "easy-c-unit.h"

#if defined(_MSC_VER ) && defined(WIN)
#   pragma comment(lib, "user32.lib")
#   define  I_AM_CALLED  MessageBox(0, "unit test sample", __FUNCSIG__, MB_OK);
#else
#   define  I_AM_CALLED  printf( "calling [%s]\n", FUNC_NAME);
#endif

UNIT_TEST(my_test_1)
{
	I_AM_CALLED;
#ifdef unix
    sleep(1);
    assert( 0 == "asdfasdffsd" );
#endif
}

UNIT_TEST(my_test_2)
{
#ifdef unix
	sleep(-1);
#endif
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
	printf("calling [%s]\n", FUNC_NAME );
	return 0;
}
#endif
