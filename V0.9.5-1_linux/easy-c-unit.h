#ifndef ANSI_C_UNIT_TEST_H_
#define ANSI_C_UNIT_TEST_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#define DLLEXPORT
typedef void (*UnitTest_FP)(void);

#define MAX_NUM_OF_TEST_CASES 1000
void register_unit_test(const char * name, int len, const char * file, int line, UnitTest_FP fp);
#ifdef _MSC_VER
#undef DLLEXPORT
#define DLLEXPORT __declspec(dllexport)
// Any code include this header automatically link corresponding lib
#if _MSC_VER == 1200
     #pragma comment(lib, "ANSI_C_UnitTest_VC6.lib")
#elif _MSC_VER == 1310
     #pragma comment(lib, "ANSI_C_UnitTest_VC2003.lib")
#elif _MSC_VER == 1400
     #pragma comment(lib, "ANSI_C_UnitTest_VC2005.lib")
#elif _MSC_VER == 1500
     #pragma comment(lib, "ANSI_C_UnitTest_VC2008.lib")
#elif _MSC_VER == 1600
     #pragma comment(lib, "ANSI_C_UnitTest_VC2010.lib")
#else
#    error Only VC6/VC2003/2005/2008/2010 supported. Your version is [_MSC_VER]!
#endif

#define CCALL __cdecl
#if _MSC_VER == 1310
	// The CRT section's default attribute is read-write in VC2003.
	// And Read Only in VC2005, 2008, 2010
#	pragma section(".CRT$XCU", read ,write)
#elif _MSC_VER == 1200
    // VC6 need to define the data segment(actually section) at first
	// But then set the default data segment to ".data"
#pragma data_seg(".CRT$XCU")
#pragma data_seg(".data")
#else
#   pragma section(".CRT$XCU", read)
#endif

#define INITIALIZER(f) \
   static void __cdecl f(void); \
   __declspec(allocate(".CRT$XCU")) void (__cdecl* EXP_LINE(f,_) )(void) = f; \
   static void __cdecl f(void)

#elif defined(__GNUC__)

#define CCALL
#define INITIALIZER(f) \
   static void f(void) __attribute__((constructor));\
   static void f(void)

#else

#error Only VC6/VC2003/2005/2008/2010 and GCC is supported!

#endif

#define EXP_LINE(a, l) a##l
#define CONCAT(a, b) EXP_LINE(a,b)
#define UNIT_TEST(a)  static void a(void);\
    INITIALIZER( CONCAT(reg_##a, __LINE__))      \
    {                                     \
        register_unit_test(#a, sizeof(#a) - 1, __FILE__, __LINE__, a);        \
    }                                     \
    void a(void)

struct UnitTestCases {
	// Indicates that unit_test_main has been called, only this function
	// set this flag to 1. In case there's no any test case. This flag
	// can indicate whether all unit test case are registered OK.
	int reg_done_;
	int num_of_cases_;
	unsigned int len_;
	struct UnitTestOneCase {
		int name_len_;
		const char * name_;
		const char * file_;
		int line_;
		UnitTest_FP fp_;
	} test_cases_[MAX_NUM_OF_TEST_CASES];
};

extern DLLEXPORT int can_exit_flag;
extern DLLEXPORT struct UnitTestCases g_unit_test_cases ;

#ifdef __cplusplus
}
#endif

#endif // ANSI_C_UNIT_TEST_H_
