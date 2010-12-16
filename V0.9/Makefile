all:
	gcc -D_GNU_SOURCE -Wl,-init,my_init -std=c99 -fPIC -shared -o libCURunnerLinux.so CURunnerLinux.c -lelf
	gcc -D_GNU_SOURCE -c -Wall -std=c99 -Werror ANSI_C_UnitTest.c
	ar rcs libeasy-c-unit.a ANSI_C_UnitTest.o
	gcc -Wl,-E -o simple_vc_gcc simple_vc_gcc.c -leasy-c-unit
install:
	mkdir -p $(DESTDIR)/usr/bin  $(DESTDIR)/usr/lib $(DESTDIR)/usr/include
	cp cunit_runner $(DESTDIR)/usr/bin/
	mkdir -p $(DESTDIR)/usr/share/doc/easy-c-unit/
	cp Readme.txt simple_vc_gcc.c $(DESTDIR)/usr/share/doc/easy-c-unit/
	cp ANSI_C_UnitTest.h $(DESTDIR)/usr/include/easy-c-unit.h
	cp libCURunnerLinux.so libeasy-c-unit.a $(DESTDIR)/usr/lib/
rpm: all
	if [ -e easy-c-unit-0.9.5 ] ; then rm -fr easy-c-unit-0.9.5; else true; fi
	@for dir in root BUILD SPECS RPMS/i386; do [ -e ~/rpm_build/$$dir ] || mkdir -p ~/rpm_build/$$dir; done
	mkdir easy-c-unit-0.9.5
	cp cunit_runner ANSI_C_UnitTest.[ch] CURunnerLinux.c simple_vc_gcc.c Readme.txt easy-c-unit.spec Makefile easy-c-unit-0.9.5
	tar zcf easy-c-unit-0.9.5.tar.gz easy-c-unit-0.9.5
	rm -fr easy-c-unit-0.9.5
	rpmbuild --buildroot ~/rpm_build/root -tb easy-c-unit-0.9.5.tar.gz
