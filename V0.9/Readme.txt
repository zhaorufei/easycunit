Easy C Unit test is a simple yet useful cross-platform unit test framwrok for
C and C++.  The main purpose is to try best to reduce the programmer's burden
to write and run unit test. Easy C Unit engaged to make unit test in C/C++
pragmatic.

Easy C Unit encourge you write Unit Test inside the source file along with 
your production code, and even without a macro(you can do it of course) to
switch off the unit test code.

The programmer's responsbility is:

1. #include <easy-c-unit.h>
============================

2. Write Unit test as
============================

UNIT_TEST(my_test_1)
{
	... your test code.
	... you can express fail by assert(3). abort(3), exit(non-0) or
	... any other abnormal ways to exit a process.
}

3. Compile your code with -Wl,-E and an additional library:
============================
gcc -Wl,-E ...   -leasy-c-unit

4. Run your unit test
============================

cunit_runner a.out 
without any argument will run all the unit test

cunit_runner a.out my_test_1 my_test_2 ...

Note: You can of course write the whole test name, but you can also uses
regexp(POSIX extended), e.g.,
cunit_runner a.out '^my_.*'

Features
========
1. Each test is run in a separate process.
2. Can catch abnormal exit by signal.
3. Test of executable or .so both supported

That's all. You need NOT to write an alternative main function to call
your unit test function and recompile. You can keep the unit test code
always in your released exectable.

Example :
/usr/share/doc/easy-c-unit/simple_vc_gcc.c

Details
========
1. The test process's working directory is the test target's directory.

2. If specified file is not a valid ELF file. cunit_runner exit 1 with
   the following error message to stderr:
   File [a.out] is not an ELF executable

3. If specified file does not contain any test cases(Even it's linked 
   with easy-c-unit library), cunit runner exit 1 with the following error
   message to stderr:
   [a.out] does not contain eacy-c-unit test cases
   cunit_runner use objdump -T to check the g_unit_test_cases symbol to
   determine whether the target file contains unit test cases.

4. When test .so file. /usr/bin/test is selected as the executable although no
   actual code of it will be executed.
