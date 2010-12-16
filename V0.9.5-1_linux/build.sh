#!/bin/bash

linker_ver=$(link | sed -rn 's/.*Linker Version ([0-9]+)\..*/\1/p')
if [ "$linker_ver" == "10" ]  ; then vc_ver=VC2010
elif [ "$linker_ver" == "9" ] ; then vc_ver=VC2008
elif [ "$linker_ver" == "8" ] ; then vc_ver=VC2005
elif [ "$linker_ver" == "7" ] ; then vc_ver=VC2003
elif [ "$linker_ver" == "6" ] ; then vc_ver=VC6
else { echo "Unsupported Linker version: $linker_ver"; exit 1; }
fi

cl /c /Zi /Fd"ANSI_C_UnitTest_$vc_ver.pdb" ANSI_C_UnitTest.c
lib /out:ANSI_C_UnitTest_$vc_ver.lib ANSI_C_UnitTest.obj
cl /Zi simple_vc_gcc.c
# Runner must be compiled with VC2010, uses tr1::regex
# cl /EHsc /Zi /J CURunner.cpp VC_ver_specific.cpp
./CUrunner simple_vc_gcc.exe
