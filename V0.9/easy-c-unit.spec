
%define disttag el5

Summary: A c/c++ unit test framework which makes unit test fantastic easy.
Name: easy-c-unit
Version: 0.9.5
Release: 1.%{disttag}
License: proprietary
Group: Development
Source0: %{name}-%{version}.tar.gz
Buildroot: %{_tmppath}/%{name}-%{version}

%description
Easy C Unit is a very simple C/C++ unit test framework. It minimize the
programmer's burden to write and run unit test in C and C++. Only support
VC and GNU toolchain at present. The following steps to write and run unit
test:
1. #include <easy-c-unit.h>
2. write unit test as: UNIT_TEST(my_test_1) {  ... }
3. Compile with      : [gcc -Wl,-E ... -leasy-c-unit]
4. Run your test with: [cunit_runner a.out]
See /usr/share/doc/easy-c-unit/Readme.txt for more details.

%prep
%setup -q

%build
make

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/usr/lib/libCURunnerLinux.so
/usr/lib/libeasy-c-unit.a
/usr/bin/cunit_runner
/usr/include/easy-c-unit.h
/usr/share/doc/easy-c-unit/Readme.txt
/usr/share/doc/easy-c-unit/simple_vc_gcc.c
