Easy C Unit test is a simple yet useful cross-platform unit test framwrok for
C and C++.  The main purpose is to try best reduce the programmer's burden to
write and run unit test, to push unit test to practice.

Easy C Unit encourge you write Unit Test inside the source file along with 
your production code, and even without a macro(you can do it of course) to
switch off the unit test code.

The programmer's responsbility is:

1. #include <easy-c-unit.h>
============================

2. Write Unit test as
============================

TEST(my_test_1)
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
without any argument will list all the unit test

cunit_runner a.out my_test_1 my_test_2 ...

That's all. You need not to write an alternative main function to call
you unit test function and recompile. You can keep the unit test code
always in your released exectable.

Example :
/usr/share/doc/easy-c-unit/simple_vc_gcc.c
