#!/bin/bash

rm *.obj *.asm
#set -x
function VC2003
{
	for lib in *.lib; do
		lib /list $lib | grep -i 'crt[^\\]*$' | while read -r obj; do
		lib "/extract:$obj" $lib
		obj_fname="$(basename $obj)"
		dumpbin /disasm:bytes "$obj_fname" | grep -i "^_[^_]*mainCRTStartup" || { rm "$obj_fname" ; continue; }
		dumpbin /disasm:bytes "${obj_fname}" > tmp.asm
		main="$(sed -rn 's/^(_[^_]*[mM]ainCRTStartup).*/\1/p' tmp.asm)"
		asm_fname="${lib%.lib}_${obj_fname%.obj}$main.asm"
		mv tmp.asm  "$asm_fname"
		sed -rn '/^_[^_]*[mM]ainCRTStartup/,/^\r?$/p' $asm_fname > a_$asm_fname
		sed -r '/^  [0-9A-F]{8}:/!d; s/^  [0-9A-F]{8}: ([0-9A-F ]*).*/\1/' a_$asm_fname | xxd -r -ps > ${asm_fname%asm}cpu
	done
done
}
function VC2008
{
	for lib in *.lib; do
		lib /list $lib | grep -i 'crt[^\\]*$' | while read -r obj; do
		lib "/extract:$obj" $lib
		obj_fname="$(basename $obj)"
		dumpbin /disasm:bytes "$obj_fname" | grep -i "mainCRTStartup:" || { rm "$obj_fname" ; continue; }
		dumpbin /disasm:bytes "${obj_fname}" > tmp.asm
		main="$(sed -rn '/[mM]ainCRTStartup/{s/^(.*[mM]ainCRTStartup).*/\1/p;q;}' tmp.asm)"
		[[ "$main" =~ "DllMain" ]] && { rm $obj_fname; continue; }

		asm_fname="${lib%.lib}_${obj_fname%.obj}$main.asm"
		mv tmp.asm  "$asm_fname"
		sed -rn '/^___tmainCRTStartup/,/^\r?$/p' $asm_fname > a_$asm_fname
		sed -r '/^  [0-9A-F]{8}:/!d; s/^  [0-9A-F]{8}: ([0-9A-F ]*).*/\1/' a_$asm_fname | xxd -r -ps > ${asm_fname%asm}cpu
	done
done
}
VC2008
