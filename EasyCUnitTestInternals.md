Present internals to design a C unit test framework to make it possible that a runner can get list of unit test and run arbitrary unit test.

# Introduction #

The current solution only works for `VC6/VC2003/VC2005/VC2008/VC2010` compiler. In the recent future gcc will be supported.

The C unit test framework take use of compiler specific features to register a unit test function to make it being called automatically before `main/WinMain`.

Please refer to [SampleUsage](SampleUsage.md) for usage.

# Details #

## Compile time and link time ##
For VC++. It's the

```
#   pragma section(".CRT$XCU", read)
```

For every unit test function, whose prototype is:
```
   void __cdecl f(void);
```
A special file scope static function pointer is defined in the above section:
```
   __declspec(allocate(".CRT$XCU")) void (__cdecl* EXP_LINE(f,_) )(void) = f;
```

Note that it's a pointer to a register function being put at ".CRT$XCU" section. The task of the register function is to register the unit test function and it's name as an entry to the collection of the unit test.

The linker will collect all the function pointers into one section. Before the `main/WinMain` being called, the CRT code will iterate all the function pointers and call them one by one.

All the ugly stuff are hidden behind a macro:
```
UNIT_TEST(f)
{
        ....
}
```

## Runtime ##
The runner will intercept the program(like a debugger) and modify the `call main/WinMain` instruction on-the-fly to make the callee program run a faked `main/WinMain` function, it's the faked function that talk to the runner return the list of the unit test, and can run specific unit test.

### How to find the `call main/WinMain` instruction to replace ###
The CURunner program itself must be compiled by VC2010 because it uses tr1 regex to find the intel machine code pattern. Regex works on binary data? Absolutely yes.