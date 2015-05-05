Although many C unit test exist and almost all of them declared that easy of use. But in reality it's not easy. This project focus on making writing and running ANSI C(Also works for C++) Unit Test as easy as possible.

1. How easy to write ANSI C unit test:
#include <easy-c-unit.h>

UNIT\_TEST(foo\_test)
{
> ...
}

2. How easy you can run the unit test:

**Do NOT need to modify main/WinMain**

**exe/DLL, or Linux executable/.so**

**CuRunner  program.exe | component.dll | linux\_elf | linux\_lib.so**

That's it!

Note that C/C++ doesn't support reflection by itself. To make a runner call arbitrary function in it, especially regardless of debug info, it's necessary to take use of compiler specific features(Not ANSI C or ANSI C++). At present, the runner can support the following compiler versions:

VC 6.0
VC 2003
VC 2005
VC 2008
VC 2010

GCC (which support attribute((constructor)) )

At present this project itself does NOT provide TEST Assertions. The technical can be used with any existing C Unit Test framework, as well as C++ unit test framework such as UnitTest++.