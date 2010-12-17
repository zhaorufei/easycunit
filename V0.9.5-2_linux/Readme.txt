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

That's all. You need NOT to write an alternative main function to call
your unit test function and recompile. You can keep the unit test code
always in your released exectable.

Example :
/usr/share/doc/easy-c-unit/simple_vc_gcc.c
